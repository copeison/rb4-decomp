#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/dsp/Ramper.h"

// Bit-depth and sample-rate reducer (audio/BitCrusher.o). The members have
// not been reconstructed; they are declared for FusionSampler. The object is
// 176 bytes.
class BitCrusher {
public:
    BitCrusher();   // 0xDA390
    ~BitCrusher();  // 0xDA450, empty

    // The map's signature is Setup(int, int). At 0xDA460.
    void Setup(int numChannels, int sampleRate);
    // The flag forces the update. At 0xDA410.
    void SetCrush(float bits, bool force);
    // A percentage; the flag marks the wet level changed. At 0xDA430.
    void SetWet(float percent, bool force);
    // Clears the held samples. At 0xDA4A0. Name not in the reference map.
    void ClearHistory();
    void SetSampleHoldFactor(unsigned short factor);           // 0xDA4C0
    void Process(AudioBuffer<float>& input, AudioBuffer<float>& output);  // 0xDA4F0

    // Field names are not in the reference map. mOpaque arrays are bytes the
    // reconstructed code does not use.
    int mNumChannels;
    int mSampleRate;
    float mCrush;  // Bits.
    float mWet;    // Percent.
    bool mWetChanged;
    SPL::Parameter mSampleHoldFactor;
    unsigned char mOpaque56[64];  // The held samples.
    short mCrushBits;
    unsigned short mHoldCounter;
    SPL::Ramper mWetRamp;
};

static_assert(offsetof(BitCrusher, mCrush) == 8);
static_assert(offsetof(BitCrusher, mWetChanged) == 16);
static_assert(offsetof(BitCrusher, mSampleHoldFactor) == 24);
static_assert(offsetof(BitCrusher, mCrushBits) == 120);
static_assert(offsetof(BitCrusher, mWetRamp) == 128);
static_assert(sizeof(BitCrusher) == 176);
