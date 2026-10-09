#include "audio/core/dsp/SmbPitchShift.h"

#include <cmath>
#include <cstring>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/decoders/AudioDecoder.h"
#include "audio/core/resources/AudioSampleResource.h"

namespace {

// Single precision, so 2 * kPi in double is 6.2831854820251465 as in the
// binary. Name not in the reference map.
const float kPi = 3.14159265358979323846f;

// The "timestretch_settings" algorithms, at 0x18E6240. Name not in the
// reference map.
const char* const kAlgorithmNames[] = {
    "as_authored",
    "default",
    "elastique_pro",
    "elastique_eff_with_formant",
    "elastique_mobile",
};

}  // namespace

bool SmbPitchShift::sTablesInitialized;
float SmbPitchShift::sWindow128[128];
float SmbPitchShift::sWindow256[256];
float SmbPitchShift::sWindow512[512];
float SmbPitchShift::sWindow1024[1024];
float SmbPitchShift::sWindow2048[2048];
float SmbPitchShift::sWindow4096[4096];

// Reconstructed from eboot.elf at 0xDEF10.
void SmbPitchShift::ChannelState::Clear() {
    std::memset(this, 0, sizeof(ChannelState));
}

// Reconstructed from eboot.elf at 0xDEF30.
void SmbPitchShift::Setup(int frameSize, int oversampling) {
    mMonoCutoffBin = 0;
    mFrameSize = frameSize;
    mHalfFrameSize = frameSize / 2;
    mOversampling = oversampling;
    mStepSize = frameSize / oversampling;
    mExpectedPhase = 2.0 * kPi * mStepSize / frameSize;
    mLatency = frameSize - mStepSize;
    if (!sTablesInitialized) {
        InitTables();
    }
    switch (frameSize) {
    case 128:
        mWindow = sWindow128;
        break;
    case 256:
        mWindow = sWindow256;
        break;
    case 512:
        mWindow = sWindow512;
        break;
    case 1024:
        mWindow = sWindow1024;
        break;
    case 2048:
        mWindow = sWindow2048;
        break;
    case 4096:
        mWindow = sWindow4096;
        break;
    default:
        break;
    }
    ClearBuffers();
}

namespace {

// Name not in the reference map.
void MakeHannWindow(float* window, int size) {
    for (int k = 0; k < size; ++k) {
        window[k] = static_cast<float>(-0.5 * cos(2.0 * kPi * k / size) + 0.5);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0xDF040, which unrolls the helper.
void SmbPitchShift::InitTables() {
    MakeHannWindow(sWindow128, 128);
    MakeHannWindow(sWindow256, 256);
    MakeHannWindow(sWindow512, 512);
    MakeHannWindow(sWindow1024, 1024);
    MakeHannWindow(sWindow2048, 2048);
    MakeHannWindow(sWindow4096, 4096);
    sTablesInitialized = true;
}

// Reconstructed from eboot.elf at 0xDF230.
void SmbPitchShift::ClearBuffers() {
    std::memset(mChannels, 0, sizeof(mChannels));
}

// Reconstructed from eboot.elf at 0xDF250.
int SmbPitchShift::GetLatency() {
    return mLatency;
}

// Reconstructed from eboot.elf at 0xDF260.
void SmbPitchShift::SetMonoCutoff(float frequency) {
    mMonoCutoffBin = static_cast<int>(frequency / static_cast<float>(mFreqPerBin));
}

// Reconstructed from eboot.elf at 0xDF280.
void SmbPitchShift::SetSampleRate(float sampleRate) {
    mFreqPerBin = static_cast<double>(sampleRate) / mFrameSize;
    ClearBuffers();
}

// Reconstructed from eboot.elf at 0xDF2B0.
void SmbPitchShift::UpdateSampleRate() {
    mFreqPerBin = static_cast<double>(mAudioData->GetSampleRate()) / mFrameSize;
    ClearBuffers();
}

// Reconstructed from eboot.elf at 0xDF300.
double SmbPitchShift::Render(
    AudioBuffer<float>& buffer,
    double position,
    double rate,
    double pitchRatio,
    double timeRatio,
    bool loop,
    const unsigned int* channelMap,
    const float* gains) {
    double next = mDecoder->Render(buffer, position, rate * timeRatio, loop, channelMap, gains);
    float pitchShift = pitchRatio / timeRatio;
    int numChannels = buffer.mNumChannels;
    float* left = buffer.mChannelData[0];
    long numFrames = buffer.mNumFrames;
    float* right;
    int stride;
    if (buffer.mConfig.mInterleaved) {
        right = left + 1;
        stride = numChannels;
    } else {
        right = buffer.mChannelData[1];
        stride = 1;
    }
    ProcessChannel(numFrames, left, left, 0, stride, pitchShift);
    if (numChannels >= 2) {
        ProcessChannel(numFrames, right, right, 1, stride, pitchShift);
    }
    return next;
}

// Reconstructed from eboot.elf at 0xDF3C0.
void SmbPitchShift::ProcessChannel(
    long numSamples,
    const float* in,
    float* out,
    int channel,
    int stride,
    float pitchShift) {
    ChannelState& state = mChannels[channel];
    if (state.mRover == 0) {
        state.mRover = mLatency;
    }
    for (long i = 0; i < numSamples; ++i) {
        state.mInFifo[state.mRover] = in[i * stride];
        out[i * stride] = state.mOutFifo[state.mRover - mLatency];
        ++state.mRover;
        if (state.mRover < mFrameSize) {
            continue;
        }
        state.mRover = mLatency;

        // Analysis.
        for (int k = 0; k < mFrameSize; ++k) {
            state.mFftWorkspace[2 * k] = mWindow[k] * state.mInFifo[k];
            state.mFftWorkspace[2 * k + 1] = 0.0f;
        }
        Fft(state.mFftWorkspace, -1);
        for (long k = channel > 0 ? mMonoCutoffBin : 0; k <= mHalfFrameSize; ++k) {
            double real = state.mFftWorkspace[2 * k];
            double imag = state.mFftWorkspace[2 * k + 1];
            double magn = 2.0 * sqrt(real * real + imag * imag);
            double phase = atan2(imag, real);
            double tmp = phase - state.mLastPhase[k];
            state.mLastPhase[k] = phase;
            tmp -= k * mExpectedPhase;
            long qpd = tmp / kPi;
            if (qpd >= 0) {
                qpd += qpd & 1;
            } else {
                qpd -= qpd & 1;
            }
            tmp -= kPi * static_cast<double>(qpd);
            tmp = mOversampling * tmp / (2.0 * kPi);
            tmp = k * mFreqPerBin + tmp * mFreqPerBin;
            mAnaMagn[k] = magn;
            mAnaFreq[k] = tmp;
        }

        // Processing.
        std::memset(mSynMagn, 0, mFrameSize * sizeof(float));
        std::memset(mSynFreq, 0, mFrameSize * sizeof(float));
        for (long k = 0; k <= mHalfFrameSize; ++k) {
            long index = static_cast<long>(k * pitchShift);
            if (index <= mHalfFrameSize) {
                mSynMagn[index] += mAnaMagn[k];
                mSynFreq[index] = mAnaFreq[k] * pitchShift;
            }
        }

        // Synthesis.
        for (long k = 0; k <= mHalfFrameSize; ++k) {
            double magn = mSynMagn[k];
            double tmp = mSynFreq[k];
            tmp -= k * mFreqPerBin;
            tmp /= mFreqPerBin;
            tmp = 2.0 * kPi * tmp / mOversampling;
            tmp += k * mExpectedPhase;
            state.mSumPhase[k] += tmp;
            double phase = state.mSumPhase[k];
            state.mFftWorkspace[2 * k] = magn * cos(phase);
            state.mFftWorkspace[2 * k + 1] = magn * sin(phase);
        }
        for (long k = mFrameSize + 2; k < 2 * mFrameSize; ++k) {
            state.mFftWorkspace[k] = 0.0f;
        }
        Fft(state.mFftWorkspace, 1);
        for (int k = 0; k < mFrameSize; ++k) {
            state.mOutputAccum[k] += 2.0 * mWindow[k] * state.mFftWorkspace[2 * k]
                / (mHalfFrameSize * mOversampling);
        }
        for (int k = 0; k < mStepSize; ++k) {
            state.mOutFifo[k] = state.mOutputAccum[k];
        }
        std::memmove(state.mOutputAccum, state.mOutputAccum + mStepSize, mFrameSize * sizeof(float));
        std::memmove(state.mInFifo, state.mInFifo + mStepSize, mLatency * sizeof(float));
    }
}

// Reconstructed from eboot.elf at 0xDF940.
void SmbPitchShift::Fft(float* buffer, long sign) {
    long size = mFrameSize;
    for (long i = 2; i < 2 * size - 2; i += 2) {
        long j = 0;
        for (long bitm = 2; bitm < 2 * size; bitm <<= 1) {
            if (i & bitm) {
                ++j;
            }
            j <<= 1;
        }
        if (i < j) {
            float* p1 = buffer + i;
            float* p2 = buffer + j;
            float temp = *p1;
            *(p1++) = *p2;
            *(p2++) = temp;
            temp = *p1;
            *p1 = *p2;
            *p2 = temp;
        }
    }
    long le = 2;
    for (long k = 0; k < static_cast<long>(log(static_cast<double>(mFrameSize)) / log(2.0) + 0.5); ++k) {
        le <<= 1;
        long le2 = le >> 1;
        float ur = 1.0f;
        float ui = 0.0f;
        float arg = kPi / (le2 >> 1);
        float wr = cosf(arg);
        float wi = sign * sinf(arg);
        for (long j = 0; j < le2; j += 2) {
            float* p1r = buffer + j;
            float* p1i = p1r + 1;
            float* p2r = p1r + le2;
            float* p2i = p2r + 1;
            for (long i = j; i < 2 * mFrameSize; i += le) {
                float tr = *p2r * ur - *p2i * ui;
                float ti = *p2r * ui + *p2i * ur;
                *p2r = *p1r - tr;
                *p2i = *p1i - ti;
                *p1r += tr;
                *p1i += ti;
                p1r += le;
                p1i += le;
                p2r += le;
                p2i += le;
            }
            float tr = ur * wr - ui * wi;
            ui = ur * wi + ui * wr;
            ur = tr;
        }
    }
}

// Reconstructed from eboot.elf at 0xDFB90. Every channel uses the first
// channel's rover; the mix's input FIFO advances after both outputs.
void SmbPitchShift::ProcessStereo(
    long numSamples,
    const float* inLeft,
    const float* inRight,
    float* outLeft,
    float* outRight,
    float pitchShift) {
    ChannelState& left = mChannels[0];
    ChannelState& right = mChannels[1];
    ChannelState& mix = mChannels[2];
    if (left.mRover == 0) {
        left.mRover = mLatency;
    }
    for (long i = 0; i < numSamples; ++i) {
        left.mInFifo[left.mRover] = inLeft[i];
        right.mInFifo[left.mRover] = inRight[i];
        mix.mInFifo[left.mRover] = (inRight[i] + inLeft[i]) * 0.5f;
        outLeft[i] = left.mOutFifo[left.mRover - mLatency];
        outRight[i] = right.mOutFifo[left.mRover - mLatency];
        ++left.mRover;
        if (left.mRover < mFrameSize) {
            continue;
        }
        left.mRover = mLatency;

        for (int channel = 0; channel < kMaxChannels; ++channel) {
            ChannelState& state = mChannels[channel];
            for (int k = 0; k < mFrameSize; ++k) {
                state.mFftWorkspace[2 * k] = mWindow[k] * state.mInFifo[k];
                state.mFftWorkspace[2 * k + 1] = 0.0f;
            }
            Fft(state.mFftWorkspace, -1);
        }

        for (int channel = 0; channel < 2; ++channel) {
            ChannelState& state = mChannels[channel];
            for (long k = 0; k <= mHalfFrameSize; ++k) {
                const ChannelState& source = mChannels[k < mMonoCutoffBin ? 2 : channel];
                double real = source.mFftWorkspace[2 * k];
                double imag = source.mFftWorkspace[2 * k + 1];
                double magn = 2.0 * sqrt(real * real + imag * imag);
                double phase = atan2(imag, real);
                double tmp = phase - state.mLastPhase[k];
                state.mLastPhase[k] = phase;
                tmp -= k * mExpectedPhase;
                long qpd = tmp / kPi;
                if (qpd >= 0) {
                    qpd += qpd & 1;
                } else {
                    qpd -= qpd & 1;
                }
                tmp -= kPi * static_cast<double>(qpd);
                tmp = mOversampling * tmp / (2.0 * kPi);
                tmp = k * mFreqPerBin + tmp * mFreqPerBin;
                mAnaMagn[k] = magn;
                mAnaFreq[k] = tmp;
            }

            std::memset(mSynMagn, 0, mFrameSize * sizeof(float));
            std::memset(mSynFreq, 0, mFrameSize * sizeof(float));
            for (long k = 0; k <= mHalfFrameSize; ++k) {
                long index = static_cast<long>(k * pitchShift);
                if (index <= mHalfFrameSize) {
                    mSynMagn[index] += mAnaMagn[k];
                    mSynFreq[index] = mAnaFreq[k] * pitchShift;
                }
            }

            for (long k = 0; k <= mHalfFrameSize; ++k) {
                double magn = mSynMagn[k];
                double tmp = mSynFreq[k];
                tmp -= k * mFreqPerBin;
                tmp /= mFreqPerBin;
                tmp = 2.0 * kPi * tmp / mOversampling;
                tmp += k * mExpectedPhase;
                state.mSumPhase[k] += tmp;
                double phase = state.mSumPhase[k];
                state.mFftWorkspace[2 * k] = magn * cos(phase);
                state.mFftWorkspace[2 * k + 1] = magn * sin(phase);
            }
            for (long k = mFrameSize + 2; k < 2 * mFrameSize; ++k) {
                state.mFftWorkspace[k] = 0.0f;
            }
            Fft(state.mFftWorkspace, 1);
            for (int k = 0; k < mFrameSize; ++k) {
                state.mOutputAccum[k] += 2.0 * mWindow[k] * state.mFftWorkspace[2 * k]
                    / (mHalfFrameSize * mOversampling);
            }
            for (int k = 0; k < mStepSize; ++k) {
                state.mOutFifo[k] = state.mOutputAccum[k];
            }
            std::memmove(
                state.mOutputAccum, state.mOutputAccum + mStepSize, mFrameSize * sizeof(float));
            std::memmove(state.mInFifo, state.mInFifo + mStepSize, mLatency * sizeof(float));
        }
        std::memmove(mix.mInFifo, mix.mInFifo + mStepSize, mLatency * sizeof(float));
    }
}

// Reconstructed from eboot.elf at 0xE0350 (complete) and 0xE0360 (deleting).
SmbPitchShift::~SmbPitchShift() {}

// Reconstructed from eboot.elf at 0xE0370.
void SmbPitchShift::SetTimeStretchMode(int, int) {}

// Reconstructed from eboot.elf at 0xE0380.
void SmbPitchShift::Reset() {}

// Reconstructed from eboot.elf at 0xE0390.
bool SmbPitchShift::HasPositionOffset() {
    return false;
}

// Reconstructed from eboot.elf at 0xE03A0.
int SmbPitchShift::GetPositionOffset() {
    return 0;
}

// Reconstructed from eboot.elf at 0xE03E0.
const char* GetTimeStretchAlgorithmName(int algorithm) {
    if (static_cast<unsigned int>(algorithm) > 4) {
        return "unrecognized";
    }
    return kAlgorithmNames[algorithm];
}

// Reconstructed from eboot.elf at 0xE0400.
int GetTimeStretchAlgorithm(const char* name) {
    if (std::strcmp(name, "as_authored") == 0) {
        return 0;
    }
    if (std::strcmp(name, "default") == 0) {
        return 1;
    }
    if (std::strcmp(name, "elastique_pro") == 0) {
        return 2;
    }
    if (std::strcmp(name, "elastique_eff_with_formant") == 0) {
        return 3;
    }
    if (std::strcmp(name, "elastique_mobile") == 0) {
        return 4;
    }
    return 1;
}
