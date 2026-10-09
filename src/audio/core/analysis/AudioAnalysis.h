#pragma once

#include <atomic>
#include <cstddef>

// The component that drives a slot's analysis settings. The binary names it
// "AnalyzerCom" in its metadata; the reference map has no such class.
class AnalyzerCom;

// Settings and results of one audio analysis slot. The FMOD analysis plugin
// ("HMX.Analysis") fills the results of the slot whose id it is given, an
// AnalyzerCom drives the settings, and RndAudioTextures copies the FFT and
// semitone results into shader textures. The eight slots are a global array
// at 0x19C9960, constructed at 0xD8840.
// The reference map has no object for this code (the binary places it
// between the audio wave-file code and BiquadFilter); the class, its file and
// all of its member names are not in the reference map.
class AudioAnalysis {
public:
    static constexpr int kNumSlots = 8;
    static constexpr int kMaxFFTBins = 2048;
    static constexpr int kMaxBands = 8;
    static constexpr int kMaxSemitones = 32;

    // One band of the band filter bank. Name not in the reference map.
    struct Band {
        Band()
            : mLowCutoff(100.0f),
              mHighCutoff(200.0f),
              mRiseMs(50.0f),
              mFallMs(250.0f),
              mDbMax(0.0f),
              mDbRange(30.0f) {}

        float mLowCutoff;   // Hz.
        float mHighCutoff;  // Hz.
        float mRiseMs;
        float mFallMs;
        float mDbMax;
        float mDbRange;
    };

    // Inlined into the slot array's initializer at 0xD8840.
    AudioAnalysis();

    // The slot, or null past the last one.
    static AudioAnalysis* Get(int slot);  // 0xD8520

    // Whether an analysis plugin instance runs on this slot.
    bool IsActive() const;  // 0xD8540

    bool VUMeterEnabled() const;               // 0xD8550
    void SetVUMeterEnabled(bool enabled);      // 0xD8560
    void SetVUMeterDbMax(float dbMax);         // 0xD8570
    void SetVUMeterDbRange(float dbRange);     // 0xD85A0
    void SetVUMeterRiseMs(float riseMs);       // 0xD85D0
    void SetVUMeterFallMs(float fallMs);       // 0xD8600
    void SetVUMeterAvgWindowMs(float windowMs);  // 0xD8630
    float VUMeterResult() const;               // 0xD8660

    bool FFTEnabled() const;                 // 0xD8670
    void SetFFTEnabled(bool enabled);        // 0xD8680
    // Rounds the bin count up to a power of two.
    void SetFFTSize(int size);               // 0xD8690
    int FFTSize() const;                     // 0xD86D0
    void SetFFTDbMax(float dbMax);           // 0xD86E0
    void SetFFTDbRange(float dbRange);       // 0xD8710
    void SetFFTRiseMs(float riseMs);         // 0xD8740
    void SetFFTFallMs(float fallMs);         // 0xD8770
    void GetFFTResults(float* results, int count) const;  // 0xD87A0

    bool BandFilterBankEnabled() const;  // 0xD87C0
    int NumBands() const;                // 0xD87D0
    void GetBandResults(float* results, int count) const;  // 0xD87E0

    bool SemitoneFilterBankEnabled() const;  // 0xD8800
    int SemitoneRange() const;               // 0xD8810
    void GetSemitoneResults(float* results, int count) const;  // 0xD8820

    static AudioAnalysis sSlots[kNumSlots];  // 0x19C9960

    // Running plugin instances.
    std::atomic<int> mActiveCount;
    AnalyzerCom* mDriver;

    bool mVUMeterEnabled;
    float mVUMeterAvgWindowMs;
    float mVUMeterRiseMs;
    float mVUMeterFallMs;
    float mVUMeterDbMax;
    float mVUMeterDbRange;

    bool mFFTEnabled;
    // The band of the bins, in Hz: 20 to 5500 initially. The analyzer DSP
    // (0x111F650) spaces its bins between them.
    float mFFTMinFreq;
    float mFFTMaxFreq;
    int mFFTSize;
    // Weights the bins by the ISO 226 equal-loudness contour at 90 phon
    // (table at 0x136B2F0).
    bool mFFTEqualLoudness;
    // Spaces the bins on the mel scale, 1127 ln(1 + f / 700), rather than
    // linearly.
    bool mFFTMelScale;
    int mFFTDspSize;
    float mFFTRiseMs;
    float mFFTFallMs;
    float mFFTDbMax;
    float mFFTDbRange;

    bool mBandFilterBankEnabled;
    int mNumBands;
    Band mBands[kMaxBands];

    bool mSemitoneFilterBankEnabled;
    int mSemitoneLowNote;  // MIDI note.
    int mSemitoneRange;
    float mSemitoneRiseMs;
    float mSemitoneFallMs;
    float mSemitoneDbMax;
    float mSemitoneDbRange;

    float mVUMeterResult;
    float mFFTResults[kMaxFFTBins];
    float mBandResults[kMaxBands];
    float mSemitoneResults[kMaxSemitones];
};

static_assert(offsetof(AudioAnalysis, mDriver) == 0x8);
static_assert(offsetof(AudioAnalysis, mVUMeterEnabled) == 0x10);
static_assert(offsetof(AudioAnalysis, mVUMeterAvgWindowMs) == 0x14);
static_assert(offsetof(AudioAnalysis, mVUMeterDbRange) == 0x24);
static_assert(offsetof(AudioAnalysis, mFFTEnabled) == 0x28);
static_assert(offsetof(AudioAnalysis, mFFTSize) == 0x34);
static_assert(offsetof(AudioAnalysis, mFFTDspSize) == 0x3C);
static_assert(offsetof(AudioAnalysis, mFFTDbRange) == 0x4C);
static_assert(offsetof(AudioAnalysis, mBandFilterBankEnabled) == 0x50);
static_assert(offsetof(AudioAnalysis, mNumBands) == 0x54);
static_assert(offsetof(AudioAnalysis, mBands) == 0x58);
static_assert(offsetof(AudioAnalysis, mSemitoneFilterBankEnabled) == 0x118);
static_assert(offsetof(AudioAnalysis, mSemitoneLowNote) == 0x11C);
static_assert(offsetof(AudioAnalysis, mSemitoneRange) == 0x120);
static_assert(offsetof(AudioAnalysis, mSemitoneRiseMs) == 0x124);
static_assert(offsetof(AudioAnalysis, mVUMeterResult) == 0x134);
static_assert(offsetof(AudioAnalysis, mFFTResults) == 0x138);
static_assert(offsetof(AudioAnalysis, mBandResults) == 0x2138);
static_assert(offsetof(AudioAnalysis, mSemitoneResults) == 0x2158);
static_assert(sizeof(AudioAnalysis::Band) == 24);
static_assert(sizeof(AudioAnalysis) == 0x21D8);
