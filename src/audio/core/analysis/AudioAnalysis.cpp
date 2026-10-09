#include "audio/core/analysis/AudioAnalysis.h"

#include <cstring>

namespace {

// Limits a setting to its range. Name not in the reference map.
float ClampSetting(float value, float low, float high) {
    if (high < value) {
        return high;
    }
    return value > low ? value : low;
}

}  // namespace

// Constructed by the initializer at 0xD8840. Name not in the reference map.
AudioAnalysis AudioAnalysis::sSlots[AudioAnalysis::kNumSlots];

// Inlined into the slot array's initializer at 0xD8840, which stores the
// active count atomically and clears the results from mVUMeterResult on.
AudioAnalysis::AudioAnalysis() {
    mActiveCount = 0;
    mDriver = nullptr;
    mVUMeterEnabled = false;
    mVUMeterAvgWindowMs = 1.0f;
    mVUMeterRiseMs = 50.0f;
    mVUMeterFallMs = 250.0f;
    mVUMeterDbMax = 0.0f;
    mVUMeterDbRange = 30.0f;
    mFFTEnabled = false;
    mUnknown2C = 20.0f;
    mUnknown30 = 5500.0f;
    mFFTSize = 256;
    mUnknown38 = false;
    mUnknown39 = false;
    mFFTDspSize = 256;
    mFFTRiseMs = 50.0f;
    mFFTFallMs = 250.0f;
    mFFTDbMax = 0.0f;
    mFFTDbRange = 30.0f;
    mBandFilterBankEnabled = false;
    mNumBands = 0;
    mSemitoneFilterBankEnabled = false;
    mSemitoneLowNote = 60;
    mSemitoneRange = 12;
    mSemitoneRiseMs = 50.0f;
    mSemitoneFallMs = 250.0f;
    mSemitoneDbMax = 0.0f;
    mSemitoneDbRange = 30.0f;
    std::memset(
        &mVUMeterResult,
        0,
        sizeof(AudioAnalysis) - offsetof(AudioAnalysis, mVUMeterResult));
}

// Reconstructed from eboot.elf at 0xD8520.
AudioAnalysis* AudioAnalysis::Get(int slot) {
    if (static_cast<unsigned int>(slot) < kNumSlots) {
        return &sSlots[slot];
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xD8540.
bool AudioAnalysis::IsActive() const {
    return mActiveCount > 0;
}

// Reconstructed from eboot.elf at 0xD8550.
bool AudioAnalysis::VUMeterEnabled() const {
    return mVUMeterEnabled;
}

// Reconstructed from eboot.elf at 0xD8560.
void AudioAnalysis::SetVUMeterEnabled(bool enabled) {
    mVUMeterEnabled = enabled;
}

// Reconstructed from eboot.elf at 0xD8570.
void AudioAnalysis::SetVUMeterDbMax(float dbMax) {
    mVUMeterDbMax = ClampSetting(dbMax, -100.0f, 100.0f);
}

// Reconstructed from eboot.elf at 0xD85A0.
void AudioAnalysis::SetVUMeterDbRange(float dbRange) {
    mVUMeterDbRange = ClampSetting(dbRange, 3.0f, 100.0f);
}

// Reconstructed from eboot.elf at 0xD85D0.
void AudioAnalysis::SetVUMeterRiseMs(float riseMs) {
    mVUMeterRiseMs = ClampSetting(riseMs, 0.0f, 5000.0f);
}

// Reconstructed from eboot.elf at 0xD8600.
void AudioAnalysis::SetVUMeterFallMs(float fallMs) {
    mVUMeterFallMs = ClampSetting(fallMs, 0.0f, 5000.0f);
}

// Reconstructed from eboot.elf at 0xD8630.
void AudioAnalysis::SetVUMeterAvgWindowMs(float windowMs) {
    mVUMeterAvgWindowMs = ClampSetting(windowMs, 0.0f, 1000.0f);
}

// Reconstructed from eboot.elf at 0xD8660.
float AudioAnalysis::VUMeterResult() const {
    return mVUMeterResult;
}

// Reconstructed from eboot.elf at 0xD8670.
bool AudioAnalysis::FFTEnabled() const {
    return mFFTEnabled;
}

// Reconstructed from eboot.elf at 0xD8680.
void AudioAnalysis::SetFFTEnabled(bool enabled) {
    mFFTEnabled = enabled;
}

// Reconstructed from eboot.elf at 0xD8690.
void AudioAnalysis::SetFFTSize(int size) {
    auto bins = static_cast<unsigned int>(size) - 1;
    bins |= bins >> 1;
    bins |= bins >> 2;
    bins |= bins >> 4;
    bins |= bins >> 8;
    bins |= bins >> 16;
    mFFTSize = static_cast<int>(bins + 1);
}

// Reconstructed from eboot.elf at 0xD86D0.
int AudioAnalysis::FFTSize() const {
    return mFFTSize;
}

// Reconstructed from eboot.elf at 0xD86E0.
void AudioAnalysis::SetFFTDbMax(float dbMax) {
    mFFTDbMax = ClampSetting(dbMax, -100.0f, 100.0f);
}

// Reconstructed from eboot.elf at 0xD8710.
void AudioAnalysis::SetFFTDbRange(float dbRange) {
    mFFTDbRange = ClampSetting(dbRange, 3.0f, 100.0f);
}

// Reconstructed from eboot.elf at 0xD8740.
void AudioAnalysis::SetFFTRiseMs(float riseMs) {
    mFFTRiseMs = ClampSetting(riseMs, 0.0f, 5000.0f);
}

// Reconstructed from eboot.elf at 0xD8770.
void AudioAnalysis::SetFFTFallMs(float fallMs) {
    mFFTFallMs = ClampSetting(fallMs, 0.0f, 5000.0f);
}

// Reconstructed from eboot.elf at 0xD87A0.
void AudioAnalysis::GetFFTResults(float* results, int count) const {
    std::memcpy(results, mFFTResults, sizeof(float) * count);
}

// Reconstructed from eboot.elf at 0xD87C0.
bool AudioAnalysis::BandFilterBankEnabled() const {
    return mBandFilterBankEnabled;
}

// Reconstructed from eboot.elf at 0xD87D0.
int AudioAnalysis::NumBands() const {
    return mNumBands;
}

// Reconstructed from eboot.elf at 0xD87E0.
void AudioAnalysis::GetBandResults(float* results, int count) const {
    std::memcpy(results, mBandResults, sizeof(float) * count);
}

// Reconstructed from eboot.elf at 0xD8800.
bool AudioAnalysis::SemitoneFilterBankEnabled() const {
    return mSemitoneFilterBankEnabled;
}

// Reconstructed from eboot.elf at 0xD8810.
int AudioAnalysis::SemitoneRange() const {
    return mSemitoneRange;
}

// Reconstructed from eboot.elf at 0xD8820.
void AudioAnalysis::GetSemitoneResults(float* results, int count) const {
    std::memcpy(results, mSemitoneResults, sizeof(float) * count);
}
