#include "audio/core/analysis/PitchDetector.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "audio/core/analysis/SndAnalysis.h"
#include "audio/core/dsp/IIRFilter.h"
#include "os/memory/MemMgr.h"

// Reconstructed from eboot.elf at 0xE2A80.
PitchDetector::PitchDetector(int sampleRate) {
    mSampleRate = 0;
    mNumBlocks = 0;
    mNumSamples = 0;
    mLevel = 0.0f;
    mDecimated = nullptr;
    mSos = nullptr;
    mFullCorrelation = nullptr;
    mEnvelope = 0.0f;
    mNoiseFloor = 5.0f;
    mComputePitch = true;
    mOpaque88[0] = 0.0f;
    mOpaque88[1] = 0.0f;
    mOpaque88[2] = 1.0f;
    mNoiseFloorRate = 0.0f;
    mNoiseFloorTime = -1.0f;
    mDecimated = static_cast<float*>(MemAlloc(1024, "mbDecimated", 16));
    mSos = static_cast<float*>(MemAlloc(1024, "mbSOS", 16));
    mFullCorrelation = static_cast<float*>(MemAlloc(1024, "mbFullCC", 16));
    SetSampleRate(sampleRate);
    // Fourth-order low-pass ahead of the decimation.
    float b[5] = {0.046583f, 0.18633f, 0.2795f, 0.18633f, 0.046583f};
    float a[5] = {1.0f, -0.7821f, 0.67998f, -0.18268f, 0.030119f};
    mDecimationFilter = new IIR4PoleFilter(b, a);
}

// Reconstructed from eboot.elf at 0xE2C50.
void PitchDetector::SetSampleRate(int sampleRate) {
    if (mSampleRate == sampleRate) {
        return;
    }
    mSampleRate = sampleRate;
    mDecimation = sampleRate / 6000;
    int decimatedRate = sampleRate / mDecimation;
    mMinPeriod = decimatedRate / 1320;
    mWindowSize = (2 * (decimatedRate / 65) + 15) & ~15;
    std::memset(mDecimated, 0, mWindowSize * sizeof(float));
    std::memset(mSos, 0, mWindowSize * sizeof(float));
    std::memset(mFullCorrelation, 0, mWindowSize * sizeof(float));
    mDecimationPhase = 0;
}

// Reconstructed from eboot.elf at 0xE2D10.
PitchDetector::~PitchDetector() {
    Deallocate();
    delete mDecimationFilter;
}

// Reconstructed from eboot.elf at 0xE2D50.
void PitchDetector::Deallocate() {
    MemFree(mDecimated);
    MemFree(mSos);
    MemFree(mFullCorrelation);
}

// Reconstructed from eboot.elf at 0xE2D80.
void PitchDetector::AnalyzeBlock(
    const char*,
    short* samples,
    int numSamples,
    float sensitivity,
    float gain,
    float& pitch,
    float& energy,
    float& level,
    float& peak) {
    // Slide the window by the number of decimated samples this block adds.
    int firstIndex = (mDecimation - mDecimationPhase) % mDecimation;
    int numNew = 0;
    int writePos = 0;
    float sum = 0.0f;
    if (numSamples != 0 && firstIndex != numSamples) {
        numNew = (numSamples - 1 - firstIndex) / mDecimation + 1;
    }
    if (numNew != 0) {
        int keep = mWindowSize - numNew;
        if (keep > 0) {
            std::memcpy(mDecimated, mDecimated + numNew, keep * sizeof(float));
            float base = mSos[numNew - 1];
            int i;
            for (i = 0; i < keep; ++i) {
                mSos[i] = mSos[i + numNew] - base;
            }
            writePos = i;
            sum = mSos[writePos - 1];
        }
    }
    if (numNew > mWindowSize) {
        int skip = (numNew - mWindowSize) * mDecimation;
        samples += skip;
        numSamples -= skip;
    }

    mDecimationFilter->Begin();
    float filtered = mDecimationFilter->FilterSlow(samples[0]) * sensitivity;
    float maxEnvelope = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        float sample = samples[i];
        float envelope = sample * sample * 9.31322554e-12f + 0.99f * mEnvelope;
        mEnvelope = envelope > 1.0f ? 1.0f : std::max(envelope, 0.0f);
        float out = mDecimationFilter->FilterSlow(sample) * sensitivity;
        filtered += (out - filtered) * 0.3f;
        if (writePos < mWindowSize && (mDecimationPhase + i) % mDecimation == 0) {
            mDecimated[writePos] = filtered;
            sum += filtered * filtered;
            mSos[writePos] = sum;
            ++writePos;
        }
        maxEnvelope = std::max(mEnvelope, maxEnvelope);
    }
    mDecimationFilter->End();
    peak = maxEnvelope;
    mDecimationPhase = (mDecimationPhase + numSamples) % mDecimation;
    mLevel = sqrtf(mSos[mWindowSize - 1]) / mWindowSize;

    if (mComputePitch) {
        ShiftedDotProduct(mDecimated, mWindowSize, mFullCorrelation, true);
        int period = FindCCPeak(mFullCorrelation, mSos, mWindowSize, mMinPeriod);
        mPeriod = RefinePeriod2(mDecimated, mSos, mFullCorrelation, mWindowSize, period);
    }

    // Ten-second time constant, cached; the rate is 1 - exp(-1 / 600).
    if (mNoiseFloorTime != 10.0f) {
        mNoiseFloorTime = 10.0f;
        mNoiseFloorRate = 0.00166529417f;
    }
    if (mNumBlocks > 60) {
        if (mLevel > 0.0f && mLevel < mNoiseFloor) {
            mNoiseFloor = mLevel;
        } else if (mPeriod == 0.0f || mLevel < mNoiseFloor) {
            mNoiseFloor += (5.0f - mNoiseFloor) * mNoiseFloorRate;
        }
    }
    mNoiseFloor = 1.0f;
    if (mLevel < 1.0f) {
        mPeriod = 0.0f;
        mLevel = 0.0f;
    } else if (mLevel > 500.0f) {
        mLevel = 50.0f;
    }

    float note = 0.0f;
    if (mPeriod != 0.0f) {
        float frequency = static_cast<float>(mSampleRate / mDecimation) / mPeriod;
        if (!(frequency > 0.0f)) {
            energy = 0.0f;
            pitch = 0.0f;
            mPitch = 0.0f;
            mPeriod = 0.0f;
            return;
        }
        // 12 * log2(frequency / 440) + 69.
        note = log10f(frequency) * 39.8631363f - 36.3763161f;
    }
    mPitch = note;
    ++mNumBlocks;
    mNumSamples += numSamples;
    pitch = note;
    energy = gain * 12.0f * mLevel / mNoiseFloor;
    level = mLevel;
}
