#pragma once

#include <cstddef>

// Autocorrelation pitch tracker (audio/PitchDetector.o). Each Mic owns one.
// The class has not been reconstructed; only what Mic uses is declared. The
// object is 112 bytes. Field names are not in the reference map; mOpaque
// arrays are bytes Mic does not use.
class PitchDetector {
public:
    explicit PitchDetector(int sampleRate);  // 0xE2A80
    ~PitchDetector();                        // 0xE2D10

    void SetSampleRate(int sampleRate);  // 0xE2C50
    // The map has AnalyzeBlock(char const*, short*, int, float, float,
    // float&, float&, float&); this build returns a fourth value. The output
    // names follow how Mic uses them and are partly guesses. At 0xE2D80.
    void AnalyzeBlock(
        const char* name,
        short* samples,
        int numSamples,
        float sensitivity,
        float gain,
        float& pitch,
        float& energy,
        float& periodicity,
        float& voicing);

    void* mDecimationFilter;  // The 100-byte filter the constructor creates.
    int mSampleRate;
    unsigned char mOpaque12[28];
    float* mDecimated;        // "mbDecimated"
    float* mSos;              // "mbSOS"
    float* mFullCorrelation;  // "mbFullCC"
    unsigned char mOpaque64[20];
    // Runs the full correlation in AnalyzeBlock; Mic sets it before every
    // analysis.
    bool mComputePitch;
    unsigned char mOpaque85[27];
};

static_assert(offsetof(PitchDetector, mSampleRate) == 8);
static_assert(offsetof(PitchDetector, mDecimated) == 40);
static_assert(offsetof(PitchDetector, mComputePitch) == 84);
static_assert(sizeof(PitchDetector) == 112);
