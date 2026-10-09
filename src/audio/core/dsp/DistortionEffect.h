#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/dsp/FIRFilter.h"
#include "audio/core/modulation/ModulatorTarget.h"
#include "utl/containers/Vector.h"

// Waveshaping distortion with three biquad filters and optional 4x
// oversampling (audio/DistortionEffect.o, 0xDCD50 to 0xDE919). The object is
// 1,336 bytes.
class DistortionEffect {
public:
    // The "distortion" settings' "type" enum, as GetAllowedTypes lists it.
    struct Settings {
        // The clipping curves, named by GetAllowedTypes. Value names are not
        // in the reference map.
        enum Type : int {
            kTypeClean = 0,       // tanh.
            kTypeWarm = 1,        // sin.
            kTypeDirty = 2,       // Hard clip to [-1, 1].
            kTypeSoft = 3,        // Cubic soft clip.
            kTypeAsymmetric = 4,  // Asymmetric tube-style curve.
        };

        // The type values with their names and empty help. At 0xDD4D0.
        static eastl::vector<AllowedValue<int>> GetAllowedTypes();
    };

    static constexpr int kNumFilters = 3;     // Name not in the reference map.
    static constexpr int kMaxChannels = 8;    // Name not in the reference map.
    static constexpr int kOversampling = 4;   // Name not in the reference map.

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
    // Filters and clips one planar channel in place. At 0xDDA50.
    void _ProcessChannel(int channel, float* samples, int numFrames);
    void Process(AudioBuffer<float>& buffer);  // 0xDE8D0

    // Field names are not in the reference map.
    int mType;          // A Settings::Type.
    float mInputGain;   // Linear.
    float mOutputGain;  // Linear.
    float mBias;        // Added before the input gain; always 0 in this build.
    bool mFilterEnabled[kNumFilters];
    // Whether a filter runs before the clipping rather than after it.
    // Nothing sets it in this build, so every filter runs after.
    bool mFilterPreClip[kNumFilters];
    BiquadFilter::Coefs mFilterCoefs[kNumFilters];
    // Per filter and channel: the last two intermediate values.
    double mFilterHistory[kNumFilters][kMaxChannels][2];
    FIRFilter32 mUpsamplers[kMaxChannels];
    FIRFilter32 mDownsamplers[kMaxChannels];
    bool mOversample;
    AudioBuffer<float> mOversampleBuffer;  // One channel of 4x frames.
    int mSampleRate;
};

static_assert(offsetof(DistortionEffect, mBias) == 12);
static_assert(offsetof(DistortionEffect, mFilterEnabled) == 16);
static_assert(offsetof(DistortionEffect, mFilterPreClip) == 19);
static_assert(offsetof(DistortionEffect, mFilterCoefs) == 24);
static_assert(offsetof(DistortionEffect, mFilterHistory) == 168);
static_assert(offsetof(DistortionEffect, mUpsamplers) == 552);
static_assert(offsetof(DistortionEffect, mDownsamplers) == 872);
static_assert(offsetof(DistortionEffect, mOversample) == 1192);
static_assert(offsetof(DistortionEffect, mOversampleBuffer) == 1200);
static_assert(offsetof(DistortionEffect, mSampleRate) == 1328);
static_assert(sizeof(DistortionEffect) == 1336);
