#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/dsp/FIRFilter.h"

// Clipping distortion with three filters and optional 4x oversampling
// (audio/DistortionEffect.o). The out-of-line members have not been
// reconstructed. The object is 1,336 bytes.
class DistortionEffect {
public:
    DistortionEffect();  // 0xDCD50
    // Inline: audio/FusionSampler.o emits it at 0x9D010, as the map's build
    // does.
    ~DistortionEffect() = default;

    // Name not in the reference map. At 0xDDA40.
    void SetSampleRate(int sampleRate);
    void SetInputGainDb(float db);   // 0xDD820
    void SetOutputGainDb(float db);  // 0xDD850
    void SetType(int type);          // 0xDD880, clamped to 0-4
    void SetupFilter(int index, const BiquadFilter::Settings& settings);  // 0xDD8A0
    void SetOversample(bool oversample);  // 0xDD920
    void Process(AudioBuffer<float>& buffer);  // 0xDE8D0

    // Field names are not in the reference map. mOpaque arrays are bytes the
    // reconstructed code does not use.
    unsigned char mOpaque0[552];  // The gains, the type and the filters.
    FIRFilter32 mUpsamplers[8];
    FIRFilter32 mDownsamplers[8];
    bool mOversample;
    AudioBuffer<float> mOversampleBuffer;
    int mSampleRate;
};

static_assert(offsetof(DistortionEffect, mUpsamplers) == 552);
static_assert(offsetof(DistortionEffect, mDownsamplers) == 872);
static_assert(offsetof(DistortionEffect, mOversampleBuffer) == 1200);
static_assert(offsetof(DistortionEffect, mSampleRate) == 1328);
static_assert(sizeof(DistortionEffect) == 1336);
