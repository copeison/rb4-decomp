#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/dsp/Ramper.h"
#include "audio/core/dsp/TempoListener.h"

// Feedback delay line (audio/Delay.o, 0xDA9F0 to 0xDCD4F), optionally synced
// to the tempo. The line is a power-of-two ring buffer per channel, read
// with linear interpolation. The gains and the delay ramp towards their
// parameters: the gains every 32 frames over 20 ms, the delay every 2 frames
// over 0.5 s. The map's Delay is not a TempoListener; its vtable holds only
// the destructors. The vtable is at 0x18E6180; the object is 512 bytes.
class Delay : public TempoListener {
public:
    Delay();  // 0xDA9F0
    // Slots 0-1 at 0xDCB80 and 0xDCC60, emitted with the vtable.
    // FusionSampler's destructor (0x96E20) inlines it.
    ~Delay() override {
        Unregister();
    }
    // Slot 2 at 0xDCA30: stores both and recomputes a synced delay.
    void OnTempoChanged(float tempo, float speed) override;

    // Allocates the line for the longest delay and resets the ramps. At
    // 0xDADD0.
    void Prepare(float sampleRate, unsigned int numChannels, float maxDelayMs);
    // A time in beats while synced. At 0xDB220.
    void SetDelaySeconds(float seconds);
    // Processes each planar channel of the buffer in place. At 0xDB510.
    void Process(AudioBuffer<float>& buffer);
    // Processes interleaved frames from input into output; DelayPlugin's read
    // callback calls it. At 0xDBEE0.
    void Process(float* input, float* output, int numFrames, int numChannels);
    void SetTempo(float tempo);  // 0xDCA80
    // Name not in the reference map. At 0xDCAD0.
    void SetSpeed(float speed);
    void SetBeatSync(bool beatSync);  // 0xDCB20
    // Empty. At 0xDCB70, which nothing references; whether the map's member
    // is static is not known.
    static void UnitTest();

    // Inline in the map's build, which emits them in
    // audio/FusionSampler.o; FusionSampler::LoadPatch inlines them here.
    void SetDryGain(float gain) {
        mDryGain.Set(gain);
    }
    void SetWetGain(float gain) {
        mWetGain.Set(gain);
    }
    void SetFeedbackGain(float gain) {
        mFeedbackGain.Set(gain);
    }
    // Jumps every ramp, the delay's included, to its target. The
    // constructor, Prepare and _ApplyNewParams inline it, as does
    // FusionSampler::LoadPatch at 0x987E0. Name not in the reference map.
    void FinishGainRamps() {
        mDelayRamp.Finish();
        mFeedbackRamp.Finish();
        mWetRamp.Finish();
        mDryRamp.Finish();
    }

    // Starts the ramps towards changed parameters, the delay's towards
    // mDelaySamples, and on the first block jumps to them. At 0xDB270.
    void _ApplyNewParams();

    // Recomputes mDelaySamples from the delay time. Inlined into every
    // setter. Name not in the reference map.
    void _UpdateDelaySamples() {
        float delay = mDelayTime;
        if (mBeatSync) {
            delay = 60.0F / mTempo * delay / mSpeed;
        }
        float samples = static_cast<float>(mSampleRate) * delay;
        mDelaySamples = samples < mMaxDelaySamples ? samples : mMaxDelaySamples;
    }
    // Restarts the ramps from the parameters at the sample rate. Inlined
    // into the constructor and Prepare. The delay ramp targets the delay
    // time, not mDelaySamples, until the next _ApplyNewParams. Name not in
    // the reference map.
    void _ResetRamps() {
        float sampleRate = static_cast<float>(mSampleRate);
        float samplePeriod = 1.0F / sampleRate;
        mDelayRamp.SetTarget(mDelayTime, nullptr, nullptr);
        mDelayRamp.mRate = sampleRate * 0.5F;
        mDelayRamp.mIncrement = samplePeriod * 4.0F;
        mWetRamp.SetTarget(mWetGain.Get(), nullptr, nullptr);
        mWetRamp.mRate = sampleRate * 0.03125F;
        mWetRamp.mIncrement = samplePeriod * 1600.0F;
        mDryRamp.SetTarget(mDryGain.Get(), nullptr, nullptr);
        mDryRamp.mRate = sampleRate * 0.03125F;
        mDryRamp.mIncrement = samplePeriod * 1600.0F;
        mFeedbackRamp.SetTarget(mFeedbackGain.Get(), nullptr, nullptr);
        mFeedbackRamp.mRate = sampleRate * 0.03125F;
        mFeedbackRamp.mIncrement = samplePeriod * 1600.0F;
    }

    // Field names are not in the reference map.
    int mNumChannels;
    int mSampleRate;
    unsigned int mExtraFrames;   // The frame the interpolation reads past the delay.
    unsigned int mBufferLength;  // Frames per channel, a power of two.
    AudioBuffer<float> mBuffer;  // The delay line.
    unsigned int mWritePos;
    unsigned int mBufferMask;
    SPL::Parameter mFeedbackGain;
    SPL::Parameter mWetGain;
    SPL::Parameter mDryGain;
    float mDelayTime;  // Seconds, or beats while synced.
    bool mBeatSync;
    SPL::Ramper mWetRamp;
    SPL::Ramper mDryRamp;
    SPL::Ramper mFeedbackRamp;
    SPL::Ramper mDelayRamp;  // In frames.
    float mMaxDelaySamples;
    float mDelaySamples;
    float mOutputGain;  // Linear; DelayPlugin sets it from decibels.
    float mTempo;
    float mSpeed;
    bool mFirstBlock;  // Set until the first block, whose ramps jump.
};

static_assert(offsetof(Delay, mNumChannels) == 0x28);
static_assert(offsetof(Delay, mBufferLength) == 0x34);
static_assert(offsetof(Delay, mBuffer) == 0x38);
static_assert(offsetof(Delay, mWritePos) == 0xB8);
static_assert(offsetof(Delay, mBufferMask) == 0xBC);
static_assert(offsetof(Delay, mFeedbackGain) == 0xC0);
static_assert(offsetof(Delay, mWetGain) == 0xE0);
static_assert(offsetof(Delay, mDryGain) == 0x100);
static_assert(offsetof(Delay, mDelayTime) == 0x120);
static_assert(offsetof(Delay, mBeatSync) == 0x124);
static_assert(offsetof(Delay, mWetRamp) == 0x128);
static_assert(offsetof(Delay, mDryRamp) == 0x158);
static_assert(offsetof(Delay, mFeedbackRamp) == 0x188);
static_assert(offsetof(Delay, mDelayRamp) == 0x1B8);
static_assert(offsetof(Delay, mMaxDelaySamples) == 0x1E8);
static_assert(offsetof(Delay, mOutputGain) == 0x1F0);
static_assert(offsetof(Delay, mFirstBlock) == 0x1FC);
static_assert(sizeof(Delay) == 0x200);
