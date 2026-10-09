#pragma once

#include <cstddef>

class IIR4PoleFilter;

// Autocorrelation pitch tracker (audio/PitchDetector.o). Each Mic owns one.
// Input is low-pass filtered, decimated to about 6 kHz and kept in a window
// whose autocorrelation SndAnalysis searches for the period. Field names are
// not in the reference map.
class PitchDetector {
public:
    explicit PitchDetector(int sampleRate);  // 0xE2A80
    ~PitchDetector();                        // 0xE2D10

    void Deallocate();                   // 0xE2D50, not called in this build.
    void SetSampleRate(int sampleRate);  // 0xE2C50
    // The map has AnalyzeBlock(char const*, short*, int, float, float,
    // float&, float&, float&); this build adds the peak output. The name is
    // not read. sensitivity scales the filtered input; gain scales the
    // energy. pitch is a MIDI note (0 when unvoiced), energy is 12 * gain *
    // level, level is the window's RMS-like level (0 below 1, 50 above 500)
    // and peak is the largest input envelope of the block. At 0xE2D80.
    void AnalyzeBlock(
        const char* name,
        short* samples,
        int numSamples,
        float sensitivity,
        float gain,
        float& pitch,
        float& energy,
        float& level,
        float& peak);

    IIR4PoleFilter* mDecimationFilter;  // 100 bytes, created by the constructor.
    int mSampleRate;
    int mWindowSize;   // Decimated samples analysed: twice the rate / 65, rounded up to 16.
    int mDecimation;   // Input samples per decimated sample: rate / 6000.
    int mMinPeriod;    // Shortest period searched: a 1,320 Hz pitch.
    unsigned int mNumBlocks;  // Blocks analysed; the noise floor tracks after 60.
    int mNumSamples;   // Input samples analysed.
    int mDecimationPhase;  // Input samples since the last decimated sample.
    float* mDecimated;        // "mbDecimated": the filtered, decimated window.
    float* mSos;              // "mbSOS": running sum of squares over the window.
    float* mFullCorrelation;  // "mbFullCC": autocorrelation of the window.
    float mEnvelope;  // Smoothed input power, clamped to [0, 1].
    float mPitch;     // MIDI note, or 0.
    float mPeriod;    // Refined period in decimated samples, or 0.
    float mLevel;
    // Tracks the lowest level while unvoiced, rising back toward 5. This
    // build overwrites it with 1 after every block, so the energy output is
    // not normalized.
    float mNoiseFloor;
    // Runs the period search in AnalyzeBlock; Mic sets it before every
    // analysis.
    bool mComputePitch;
    // The constructor stores 0, 0 and 1 here; nothing in this build reads
    // them.
    float mOpaque88[3];
    float mNoiseFloorRate;  // Per-block rise of mNoiseFloor.
    float mNoiseFloorTime;  // The time constant mNoiseFloorRate was set for; -1 initially.
};

static_assert(offsetof(PitchDetector, mSampleRate) == 8);
static_assert(offsetof(PitchDetector, mDecimationPhase) == 32);
static_assert(offsetof(PitchDetector, mDecimated) == 40);
static_assert(offsetof(PitchDetector, mEnvelope) == 64);
static_assert(offsetof(PitchDetector, mNoiseFloor) == 80);
static_assert(offsetof(PitchDetector, mComputePitch) == 84);
static_assert(offsetof(PitchDetector, mOpaque88) == 88);
static_assert(offsetof(PitchDetector, mNoiseFloorRate) == 100);
static_assert(sizeof(PitchDetector) == 112);
