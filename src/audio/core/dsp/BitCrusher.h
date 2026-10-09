#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/dsp/Ramper.h"

// Bit-depth and sample-rate reducer (audio/BitCrusher.o, 0xDA390 to
// 0xDA9E5). Each channel's samples are clamped to [-1, 1], truncated to 16
// bits with the low "crush" bits cleared, and held for the sample-and-hold
// factor's samples. The wet level ramps over 2 ms. The object is 176 bytes.
class BitCrusher {
public:
    // The most channels the held samples cover. Name not in the reference
    // map.
    static constexpr int kMaxChannels = 16;

    BitCrusher();   // 0xDA390
    ~BitCrusher();  // 0xDA450, empty

    // The map's signature is Setup(int, int). At 0xDA460.
    void Setup(int numChannels, int sampleRate);
    // The low bits to clear; the flag forces the update. At 0xDA410.
    void SetCrush(float bits, bool force);
    // A percentage. The flag makes the wet level jump to it instead of
    // ramping. At 0xDA430.
    void SetWet(float percent, bool jump);
    // Clears the held samples. At 0xDA4A0. Name not in the reference map.
    void ClearHistory();
    void SetSampleHoldFactor(unsigned short factor);           // 0xDA4C0
    void Process(AudioBuffer<float>& input, AudioBuffer<float>& output);  // 0xDA4F0

    // Field names are not in the reference map.
    int mNumChannels;
    int mSampleRate;
    float mCrush;  // Bits.
    float mWet;    // Percent.
    // Set by the constructor and by SetWet's flag; nothing clears it, so
    // after the first update the wet level jumps rather than ramps.
    bool mJumpWet;
    SPL::Parameter mSampleHoldFactor;  // 1 to 16 samples.
    float mHeldSamples[kMaxChannels];  // Each channel's crushed sample.
    short mCrushBits;
    unsigned short mHoldCounter;  // Samples since the held sample changed.
    SPL::Ramper mWetRamp;         // 0 to 1.
};

static_assert(offsetof(BitCrusher, mCrush) == 8);
static_assert(offsetof(BitCrusher, mJumpWet) == 16);
static_assert(offsetof(BitCrusher, mSampleHoldFactor) == 24);
static_assert(offsetof(BitCrusher, mHeldSamples) == 56);
static_assert(offsetof(BitCrusher, mCrushBits) == 120);
static_assert(offsetof(BitCrusher, mHoldCounter) == 122);
static_assert(offsetof(BitCrusher, mWetRamp) == 128);
static_assert(sizeof(BitCrusher) == 176);
