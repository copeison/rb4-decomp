#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/output/AudioRenderTarget.h"

// Generator that renders an AudioBus source into its render target's mixer.
// _InitTypeId at 0xE1450 registers the type name "AudioBusGenerator"; the
// map has no object for it. The primary vtable is at 0x18E6278 and the
// AudioBusCallable vtable at 0x18E63A8. The object is 392 bytes. Its own
// methods have not been reconstructed.
class AudioBusGenerator : public AudioGenerator, public AudioBusCallable {
public:
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
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // slot 33: 0xE14D0
    bool _MakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // slot 34: 0xE14E0
    // Slot 35 at 0xE13F0: forwards to the source bus. Name not in the
    // reference map.
    virtual void Unknown35();

    // Field names are not in the reference map.
    AudioMixer* mMixer;                // The render target's mixer.
    unsigned char mUnknown128[24];
    float* mChannelData[8];            // Rendered planar channels.
    unsigned char mUnknown216[37];
    bool mHasSamples;                  // Set once a block has been rendered.
    AudioBus* mSource;
    unsigned char mUnknown264[20];
    float mGainFadeStep;               // Gain change per rendered block.
    float mUnknown288;
    float mBlocksPerSecond;
    unsigned char mUnknown296[24];
    int mGainFadeMode;                 // 1 while a fade to silence runs.
    unsigned char mUnknown324[60];
    AudioBusCallable* mCallback;
};

static_assert(offsetof(AudioBusGenerator, mMakeGuard) == 88);
static_assert(offsetof(AudioBusGenerator, mMixer) == 120);
static_assert(offsetof(AudioBusGenerator, mGainFadeStep) == 284);
static_assert(offsetof(AudioBusGenerator, mBlocksPerSecond) == 292);
static_assert(offsetof(AudioBusGenerator, mGainFadeMode) == 320);
static_assert(offsetof(AudioBusGenerator, mChannelData) == 152);
static_assert(offsetof(AudioBusGenerator, mHasSamples) == 253);
static_assert(offsetof(AudioBusGenerator, mSource) == 256);
static_assert(offsetof(AudioBusGenerator, mCallback) == 384);
static_assert(sizeof(AudioBusGenerator) == 392);
