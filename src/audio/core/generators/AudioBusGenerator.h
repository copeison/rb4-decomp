#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/output/AudioRenderTarget.h"

// Linear ramp advanced once per rendered block. SetTarget computes the
// per-block increment from the step, the fraction of the ramp covered by
// one block. Its members are inlined at every use, for example in
// AudioBusGenerator::SetGain at 0xE1260. Names not in the reference map.
struct BlockRamp {
    BlockRamp() : mStep(1.0F), mProgress(1.0F), mBlocksPerSecond(1.0F), mBusy(false), mOnDone(nullptr) {}

    // A zero duration covers the ramp in one block.
    void SetDurationMs(float ms, float blocksPerSecond) {
        mBlocksPerSecond = blocksPerSecond;
        mStep = ms == 0.0F ? 1.0F : 1000.0F / (blocksPerSecond * ms);
    }
    void SetTarget(float target) {
        mBusy = true;
        mIncrement = (target - mValue) * mStep;
        mTarget = target;
        mProgress = 0.0F;
        mBusy = false;
        mOnDone = nullptr;
        mOnDoneContext = nullptr;
    }
    // Jumps to the target without running the completion callback.
    void Snap() {
        if (!mBusy) {
            mValue = mTarget;
            mProgress = 1.0F;
            mOnDone = nullptr;
            mOnDoneContext = nullptr;
        }
    }

    float mValue;
    float mTarget;
    float mIncrement;
    float mStep;
    float mProgress;
    float mBlocksPerSecond;
    bool mBusy;
    void (*mOnDone)(void* context);
    void* mOnDoneContext;
};

static_assert(offsetof(BlockRamp, mBlocksPerSecond) == 20);
static_assert(offsetof(BlockRamp, mBusy) == 24);
static_assert(offsetof(BlockRamp, mOnDone) == 32);
static_assert(sizeof(BlockRamp) == 48);

// Generator that renders an AudioBus source into its render target's mixer.
// _InitTypeId at 0xE1450 registers the type name "AudioBusGenerator"; the
// map has no object for it, and its gain and mute members are on
// FmodAudioBusGenerator there. The primary vtable is at 0x18E6278 and the
// AudioBusCallable vtable at 0x18E63A8. The object is 392 bytes.
class AudioBusGenerator : public AudioGenerator, public AudioBusCallable {
public:
    // The block buffer. A rendered block sets mHasSamples, which the derived
    // buffer places in the base's tail padding. Name not in the reference
    // map.
    struct RenderBuffer : public AudioBuffer<float> {
        bool mHasSamples;
    };

    explicit AudioBusGenerator(AudioBus* source);  // 0xE0490
    ~AudioBusGenerator() override;                 // slots 20-21: 0xE0A60, 0xE0C20

    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0xE1260
    float GetGain() const override;                // slot 13: 0xE1430
    void SetMute(bool mute, bool immediate) override;  // slot 14: 0xE1360
    bool GetMute() const override;                 // slot 15: 0xE1440
    void _InitTypeId() override;                   // slot 28: 0xE1450
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0xE14A0
    Symbol GetTypeId() override;                   // slot 31: 0xE14C0

    // Slot 32 at 0xE0C70. Binds the target mixer and the source, then applies
    // the requested start state and gain.
    virtual bool Setup(AudioBus* source, const PlayArgs& args, AudioBusCallable* callback);
    // Slots 33-35 override AudioBusCallable; the callable vtable's copies are
    // at 0xE14F0, 0xE1500 and 0xE1410.
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // slot 33: 0xE14D0
    bool _MakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // slot 34: 0xE14E0
    // Slot 35 at 0xE13F0: forwards to the source bus.
    bool Unknown4() override;

    // Snaps the gain and mute ramps. At 0xE0900. The map has
    // _ResetGainAndMute() on FmodAudioBusGenerator; this build passes the
    // values.
    void _ResetGainAndMute(float gain, bool mute);

    // The source's block rate, or the engine's buffer rate without a source.
    // Inlined at every ramp setup. Name not in the reference map.
    float BlocksPerSecond() const {
        if (mSource != nullptr) {
            return static_cast<float>(mSource->mSampleRate) /
                static_cast<float>(static_cast<unsigned int>(mSource->mBlockSize));
        }
        return Audio::sBuffersPerSecond;
    }

    // At 0x19E26B0. Name not in the reference map.
    static Symbol sTypeId;

    // Field names are not in the reference map.
    AudioMixer* mMixer;  // The render target's mixer.
    RenderBuffer mBuffer;
    AudioBus* mSource;
    bool mUnknown264;    // Set by Setup.
    BlockRamp mGainRamp;
    int mGainFadeMode;   // The PostFadeOption of the last SetGain.
    BlockRamp mMuteRamp;
    bool mMuted;
    AudioBusCallable* mCallback;
};

static_assert(offsetof(AudioBusGenerator, mMakeGuard) == 88);
static_assert(offsetof(AudioBusGenerator, mMixer) == 120);
static_assert(offsetof(AudioBusGenerator, mBuffer) == 128);
static_assert(offsetof(AudioBusGenerator, mBuffer.mChannelData) == 152);
static_assert(offsetof(AudioBusGenerator, mBuffer.mHasSamples) == 253);
static_assert(offsetof(AudioBusGenerator, mSource) == 256);
static_assert(offsetof(AudioBusGenerator, mUnknown264) == 264);
static_assert(offsetof(AudioBusGenerator, mGainRamp) == 272);
static_assert(offsetof(AudioBusGenerator, mGainFadeMode) == 320);
static_assert(offsetof(AudioBusGenerator, mMuteRamp) == 328);
static_assert(offsetof(AudioBusGenerator, mMuted) == 376);
static_assert(offsetof(AudioBusGenerator, mCallback) == 384);
static_assert(sizeof(AudioBusGenerator) == 392);
