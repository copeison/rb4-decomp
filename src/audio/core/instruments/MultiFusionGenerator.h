#pragma once

#include <cmath>
#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/resources/MultiFusionResource.h"
#include "audio/fmod/playback/FmodAudioBusGenerator.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"
#include "utl/text/Symbol.h"

// An instrument that routes each of 16 MIDI channels to its own instrument
// generator and plays through a bus voice of gAudioBusGeneratorManager
// (audio/MultiInstrumentGenerator.o, the map's MultiInstrumentGenerator).
// The channels' instruments are Fusion generators taken for the patches of a
// MultiFusionResource; each channel keeps a name, a volume in decibels and
// a mute flag. The primary vtable at 0x18E11A0 has 75 slots:
// InstrumentGenerator's 51 and the overrides of AudioGenerator and
// AudioBusCallable functions, which take entries after them in declaration
// order (slots 51-74). The AudioGenerator vtable is at 0x18E1408 and the
// AudioBusCallable vtable at 0x18E1518. The object is 840 bytes.
class MultiFusionGenerator : public InstrumentGenerator, public AudioBusCallable {
public:
    using ClientList =
        LinkedListSizeTracked::List<AudioBusCallable, &AudioBusCallable::_mCallbackNode>;

    static constexpr int kNumChannels = 16;  // Name not in the reference map.

    // Inlined into MultiFusionGeneratorManager::_InitGeneratorPool (0x51930):
    // VirtualInstrument(nullptr) at 0x66DF0, the inline AudioGenerator and
    // AudioBusCallable constructors and the members. The channels are left
    // for Init.
    MultiFusionGenerator()
        : InstrumentGenerator(nullptr), mTimeStretchAlgorithm(0), mTimeStretchFormantMode(0) {}
    // Slots 0-1 at 0x4F790 and 0x4FA00. The channels' instruments are not
    // released.
    ~MultiFusionGenerator() override;

    // AudioBus and VirtualInstrument overrides. The MIDI members pass the
    // call to the instrument of the channel they name.
    // Slot 4 at 0x512C0: renders every channel's instrument that is not a
    // slave into the bus buffer and adds those that are not silent. The
    // binary returns whatever the bus unlock leaves.
    bool Process(AudioBuffer<float>& buffer) override;
    // Slot 10 at 0x51D60, also AudioGenerator's slot 17 (0x43FC0).
    bool IsInstrument() override {
        return true;
    }
    // Slot 13 at 0x501E0: also clears the channels' volume and mute.
    void ResetInstrumentState() override;
    void ResetMidiState() override;  // slot 14: 0x50260
    // Slots 15-17 at 0x51D90 through 0x51DF0.
    void NoteOn(signed char note, signed char velocity, signed char channel, float startOffsetMs) override {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->NoteOn(note, velocity, channel, startOffsetMs);
        }
    }
    bool IsNotePlaying(signed char note, signed char channel) override {
        InstrumentGenerator* instrument = mInstruments[channel];
        return instrument != nullptr ? instrument->IsNotePlaying(note, channel) : false;
    }
    void NoteOff(signed char note, signed char channel) override {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->NoteOff(note, channel);
        }
    }
    // Slot 18 at 0x503C0: channel -1 reaches every instrument.
    void SetExtraPitchBend(float bend, signed char channel) override;
    // Slots 19-22 at 0x51E20 through 0x51EB0.
    void SetPitchBend(float bend, signed char channel) override {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->SetPitchBend(bend, channel);
        }
    }
    float GetPitchBend(signed char channel) const override {
        InstrumentGenerator* instrument = mInstruments[channel];
        return instrument != nullptr ? instrument->GetPitchBend(channel) : 0.0F;
    }
    void SetController(ControllerID controller, signed char msb, signed char lsb, signed char channel) override {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->SetController(controller, msb, lsb, channel);
        }
    }
    void GetController(
        ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const override {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->GetController(controller, msb, lsb, channel);
            return;
        }
        lsb = 0;
        msb = 0;
    }
    using VirtualInstrument::GetController;
    using VirtualInstrument::SetController;
    // Slots 25-26 at 0x50750 and 0x508C0 reach every instrument.
    void KillAllVoices() override;
    void AllNotesOff() override;
    void SetTempo(float tempo) override;  // slot 28: 0x51050
    // Slot 31 at 0x50700: the instruments' voices in use.
    int GetNumVoicesInUse() const override;
    // Slots 32-37 at 0x50560 through 0x506F0 keep each channel's volume, in
    // decibels, and mute, and apply them to its instrument's gain and mute.
    void SetMidiChannelVolume(float volumeDb, float fadeSecs, signed char channel) override;
    float GetMidiChannelVolume(signed char channel) const override {
        return mVolumes[channel];
    }
    void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) override;
    float GetMidiChannelGain(signed char channel) const override {
        return powf(10.0F, mVolumes[channel] * 0.05F);
    }
    void SetMidiChannelMute(bool mute, signed char channel) override;
    bool GetMidiChannelMute(signed char channel) const override {
        return mMutes[channel];
    }

    // InstrumentGenerator overrides. Slots 40 and 42-46 keep the base's
    // defaults.
    void AddAudioThreadClient(AudioBusCallable* client) override;     // slot 38: 0x4F4B0
    void RemoveAudioThreadClient(AudioBusCallable* client) override;  // slot 39: 0x4F520
    // Slot 41 at 0x51F00: loads the patch on the channel's instrument.
    void SetPatch(const ResourcePtr<FusionPatchResource>& patch, int channel) override {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->SetPatch(patch, channel);
        }
    }
    // Slot 47 at 0x50FA0: kept for instruments set later.
    void SetTranspose(int semitones) override;
    int GetTranspose() const override {  // slot 48: 0x51F70
        return mTranspose;
    }
    // Slot 49 at 0x50FF0: kept for instruments set later.
    void SetTimeStretchMode(int algorithm, int formantMode) override;
    // Slot 50 at 0x51F80: true only when both are set.
    bool GetTimeStretchMode(int* algorithm, int* formantMode) const override {
        *algorithm = mTimeStretchAlgorithm;
        *formantMode = mTimeStretchFormantMode;
        return mTimeStretchAlgorithm != 0 && mTimeStretchFormantMode != 0;
    }

    // AudioGenerator and AudioBusCallable overrides, in their primary slots
    // 51-74. The AudioGenerator vtable reaches them through this-adjusting
    // copies.
    // Slot 51 at 0x51FA0 (copy 0x52170).
    AudioGenerator* GetGeneratorOfType(Symbol type) override {
        return type == kTypeId ? this : nullptr;
    }
    // Slot 52 at 0x51FC0 (copy 0x52190).
    Symbol GetTypeId() override {
        return kTypeId;
    }
    // Slot 53 at 0x51FD0 (copy 0x52120).
    void _InitTypeId() override {
        kTypeId = Symbol("MultiFusionGenerator");
    }
    // Slot 54 at 0x4EE40 (copy 0x4F220): clears the channels, the transpose,
    // the time-stretch mode and the bus voice and prepares the bus for
    // stereo 128-sample blocks.
    void Init(AudioGeneratorManager* manager, int index) override;
    void Pause() override;                     // slot 55: 0x4FCD0 (copy 0x4FD00)
    void Continue() override;                  // slot 56: 0x4FD30 (copy 0x4FD60)
    void Stop() override;                      // slot 57: 0x4FDF0 (copy 0x4FE20)
    float GetElapsedMs() override;             // slot 58: 0x4FD90 (copy 0x4FDA0)
    float GetTimelineMs() override;            // slot 59: 0x4FDB0 (copy 0x4FDC0)
    void SeekToMs(float ms) override;          // slot 60: 0x4FDD0 (copy 0x4FDE0)
    // Slots 61-64 at 0x52020 through 0x52080 (copies 0x520A0 through
    // 0x52100) forward to the bus voice.
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override {
        if (mBusGenerator != nullptr) {
            mBusGenerator->SetGain(gain, fadeSecs, option);
        }
    }
    float GetGain() const override {
        return mBusGenerator != nullptr ? mBusGenerator->GetGain() : 0.0F;
    }
    void SetMute(bool mute, bool immediate) override {
        if (mBusGenerator != nullptr) {
            mBusGenerator->SetMute(mute, immediate);
        }
    }
    bool GetMute() const override {
        return mBusGenerator != nullptr ? mBusGenerator->GetMute() : false;
    }
    // Slot 65 at 0x510A0 (copy 0x51130): whether the bus voice or any
    // instrument took the value; every one is offered it.
    bool SetParameter(Symbol name, float value) override;
    // Slot 66 at 0x511C0 (copy 0x51240): the bus voice's value, or the
    // first instrument's that has it.
    bool GetParameter(Symbol name, float& value) override;
    // Slots 67-68 at 0x50A30 and 0x50AF0 (copies 0x50A90 and 0x50BF0): the
    // instruments' speed; the first instrument reports it, or one without
    // instruments.
    void SetSpeed(float speed, bool immediate) override;
    float GetSpeed(bool* changing) override;
    // Slot 69 at 0x4FA80 (copy 0x4FCC0): polls the instruments; false once
    // the bus voice finishes, which is kept until Release.
    bool Poll() override;
    // Slot 70 at 0x4FEB0 (copy 0x500C0): drops the clients, releases the
    // instruments, the bus voice and the resource and returns the generator
    // to its pool.
    void Release() override;
    // Slot 71 at 0x4F5B0 (AudioBusCallable copy 0x4F6A0): prepares each
    // audio-thread client and drops those that return false.
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;
    // Slots 72-73 at 0x50CF0 and 0x50DB0 (copies 0x50D50 and 0x50EB0): the
    // instruments' play scale, reported by the first instrument or one.
    void SetPlayScale(float scale) override;
    float GetPlayScale() override;
    // Slot 74 at 0x4FE50 (copy 0x4FE80): kills the bus voice, which is kept
    // until Release.
    void Kill() override;

    // Takes a bus voice for the render target and emitter and starts it on
    // this instrument, paused or playing as requested. At 0x4EDA0;
    // MultiFusionGeneratorManager::Play and the overload inline it. The map
    // has Setup(PlayArgs const&, bool).
    bool Setup(const PlayArgs& args);
    // Setup, then takes the resource's patches. At 0x4EC90.
    bool Setup(const PlayArgs& args, MultiFusionResource* resource);
    // Releases the channels' instruments and, for a loaded resource, takes a
    // Fusion generator for each channel's patch until none is free, making
    // the other channels slaves of channel 0 when the resource asks. At
    // 0x4F230. Name not in the reference map.
    void SetResource(const ResourcePtr<MultiFusionResource>& resource);
    // Puts the instrument on the channel and releases the channel's
    // previous instrument. The instrument takes the generator's emitter, the
    // channel's volume and mute, and the generator's transpose,
    // time-stretch mode and play scale. No caller in this build. At 0x500D0.
    void SetInstrument(int channel, InstrumentGenerator* instrument);
    // The channel with the name, or -1. No caller in this build. At 0x50450.
    int FindInstrumentChannel(Symbol name) const;

    static Symbol kTypeId;  // 0x19C8728, "MultiFusionGenerator"

    // Field names are not in the reference map.
    InstrumentGenerator* mInstruments[kNumChannels];
    Symbol mNames[kNumChannels];
    float mVolumes[kNumChannels];  // Decibels.
    bool mMutes[kNumChannels];
    int mTranspose;
    int mTimeStretchAlgorithm;
    int mTimeStretchFormantMode;
    AudioBusGenerator* mBusGenerator;
    ClientList mAudioThreadClients;
    CritSec mClientListLock;
    ResourcePtr<MultiFusionResource> mResource;
};

static_assert(offsetof(MultiFusionGenerator, mManager) == 0x138 + 16);
static_assert(offsetof(MultiFusionGenerator, mMakeGuard) == 0x190);
static_assert(offsetof(MultiFusionGenerator, mInstruments) == 0x1B0);
static_assert(offsetof(MultiFusionGenerator, mNames) == 0x230);
static_assert(offsetof(MultiFusionGenerator, mVolumes) == 0x2B0);
static_assert(offsetof(MultiFusionGenerator, mMutes) == 0x2F0);
static_assert(offsetof(MultiFusionGenerator, mTranspose) == 0x300);
static_assert(offsetof(MultiFusionGenerator, mTimeStretchFormantMode) == 0x308);
static_assert(offsetof(MultiFusionGenerator, mBusGenerator) == 0x310);
static_assert(offsetof(MultiFusionGenerator, mAudioThreadClients) == 0x318);
static_assert(offsetof(MultiFusionGenerator, mClientListLock) == 0x330);
static_assert(offsetof(MultiFusionGenerator, mResource) == 0x340);
static_assert(sizeof(MultiFusionGenerator) == 0x348);

// The pool of MultiFusion generators, and the registry of MultiFusion
// resources by sound name (the map's MultiInstrumentGeneratorManager; this
// build registers it as "MultiFusionGeneratorManager"). The vtable is at
// 0x18E1550. The object is 72 bytes.
class MultiFusionGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into InitGeneratorManager<MultiFusionGeneratorManager>
    // (0x66E0).
    explicit MultiFusionGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The manager's id, interned on first use. Inlined into GetId (0x51510);
    // the static is at 0x19C8730.
    static Symbol Id() {
        static Symbol id("");
        if (id == Symbol("")) {
            id = Symbol("MultiFusionGeneratorManager");
        }
        return id;
    }
    // The resource extension, interned on first use. Inlined into
    // GetResourceExt (0x515B0); the static is at 0x19C8740. Name not in the
    // reference map.
    static Symbol ResExt() {
        static Symbol resExt("");
        if (resExt == Symbol("")) {
            resExt = Symbol(".multifusion");
        }
        return resExt;
    }

    // Slot 0 at 0x4E970. Plays a registered resource's sound, or a bare
    // instrument for the name ".multifusion".
    AudioGenerator* Play(const PlayArgs& args) override;
    void Init() override;                  // slot 3: 0x4E790
    int GetIndex() override;               // slot 6: 0x51500
    Symbol GetId() override;               // slot 7: 0x51510
    Symbol GetResourceExt() override;      // slot 8: 0x515B0
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x51650
    void SendStopToAllGenerators() override;  // slot 10: 0x516E0
    void SendKillToAllGenerators() override;  // slot 11: 0x51750
    void GetActiveHandles(void* handles) override;  // slot 12: 0x517D0
    void _SetManagerIndex(int index) override;      // slot 13: 0x51920
    void _InitGeneratorPool() override;    // slot 14: 0x51930
    bool _DeleteGeneratorPool() override;  // slot 15: 0x51C80
    ~MultiFusionGeneratorManager() override;  // slots 16-17: 0x51D30, 0x51D40

    // Maps the sound name to the resource. At 0x4E7A0; the resource's loaders
    // (0x65D0E, 0x6610D) call it. Name not in the reference map.
    static void RegisterMultiFusionResource(Symbol name, MultiFusionResource* resource);
    // Removes every name that maps to the resource; false when none did. At
    // 0x4E8A0. Name not in the reference map.
    static bool UnregisterMultiFusionResource(MultiFusionResource* resource);

    static const char* kIdStr;  // 0x19B00C8

    // Sound names to resources. At 0x19C86E0. Name not in the reference map.
    static eastl::map<Symbol, MultiFusionResource*> mResourceMap;
    // Guards mResourceMap. At 0x19C86D0. Name not in the reference map.
    static CritSec mResourceMapLock;

    MultiFusionGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(MultiFusionGeneratorManager, mPool) == 64);
static_assert(sizeof(MultiFusionGeneratorManager) == 72);
