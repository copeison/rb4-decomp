#pragma once

#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/fmod/playback/FmodAudioBusGenerator.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// A rack of instruments played as one through a bus voice of
// gAudioBusGeneratorManager (audio/SynthRackGenerator.o). Each slot of the
// rack holds an instrument generator with a name, a channel volume and a
// mute flag; the rack renders the instruments into its own bus buffer and
// mixes them. The class is newer than the reference map: SetInstrument and
// FindInstrumentChannel take the names of the map's MultiInstrumentGenerator,
// and the other names are inferred. The primary vtable at 0x18E21E8 has 75
// slots: InstrumentGenerator's 51 and the overrides of AudioGenerator and
// AudioBusCallable functions, which take entries after them in declaration
// order (slots 51-74). The AudioGenerator vtable is at 0x18E2450 and the
// AudioBusCallable vtable at 0x18E2560. The object is 528 bytes.
class SynthRackGenerator : public InstrumentGenerator, public AudioBusCallable {
public:
    using ClientList =
        LinkedListSizeTracked::List<AudioBusCallable, &AudioBusCallable::_mCallbackNode>;

    // One instrument of the rack. Its default constructor is inlined into
    // the vector growth at 0x5B0E0. Names not in the reference map.
    struct RackSlot {
        RackSlot() : mInstrument(nullptr), mName(""), mVolume(0.0F), mMute(false) {}

        InstrumentGenerator* mInstrument;
        Symbol mName;
        // The channel volume and mute an instrument takes when it is set;
        // ResetInstrumentState clears them.
        float mVolume;
        bool mMute;
    };

    // Inlined into SynthRackGeneratorManager::_InitGeneratorPool (0x5A9C0):
    // VirtualInstrument(nullptr) at 0x66DF0, the inline AudioGenerator and
    // AudioBusCallable constructors and the members. The transpose and the
    // bus voice are left for Init.
    SynthRackGenerator()
        : InstrumentGenerator(nullptr),
          mBeat(0.0F),
          mTimeStretchAlgorithm(0),
          mTimeStretchFormantMode(0) {}
    // Slots 0-1 at 0x587F0 and 0x58A60. The instruments are not released.
    ~SynthRackGenerator() override;

    // AudioBus and VirtualInstrument overrides.
    // Slot 4 at 0x5A340: renders each instrument into the rack's bus buffer
    // at the rack's beat and adds those that are not silent. The binary
    // returns whatever the bus unlock leaves.
    bool Process(AudioBuffer<float>& buffer) override;
    // Slot 10 at 0x5ADB0, also AudioGenerator's slot 17 (0x43FC0).
    bool IsInstrument() override {
        return true;
    }
    // Slots 13-14 at 0x59170 and 0x59210 forward to every instrument;
    // ResetInstrumentState also clears the slots' volume and mute.
    void ResetInstrumentState() override;
    void ResetMidiState() override;
    // Slots 15-22 at 0x5ADC0 through 0x5AE20: the rack plays no notes of its
    // own; the indexed overloads below reach its instruments.
    void NoteOn(signed char note, signed char velocity, signed char channel, float startOffsetMs) override {
        static_cast<void>(note);
        static_cast<void>(velocity);
        static_cast<void>(channel);
        static_cast<void>(startOffsetMs);
    }
    bool IsNotePlaying(signed char note, signed char channel) override {
        static_cast<void>(note);
        static_cast<void>(channel);
        return false;
    }
    void NoteOff(signed char note, signed char channel) override {
        static_cast<void>(note);
        static_cast<void>(channel);
    }
    void SetPitchBend(float bend, signed char channel) override {
        static_cast<void>(bend);
        static_cast<void>(channel);
    }
    float GetPitchBend(signed char channel) const override {
        static_cast<void>(channel);
        return 1.0F;
    }
    void SetController(ControllerID controller, signed char msb, signed char lsb, signed char channel) override {
        static_cast<void>(controller);
        static_cast<void>(msb);
        static_cast<void>(lsb);
        static_cast<void>(channel);
    }
    void GetController(
        ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const override {
        static_cast<void>(controller);
        static_cast<void>(channel);
        lsb = 0;
        msb = 0;
    }
    using VirtualInstrument::GetController;
    using VirtualInstrument::HandleMidiMessage;
    using VirtualInstrument::SetController;
    // Slots 25-26 at 0x59BD0 and 0x59C20 forward to every instrument.
    void KillAllVoices() override;
    void AllNotesOff() override;
    // Slot 27 at 0x5AE30: the beat the instruments render at.
    void SetBeat(float beat) override {
        mBeat = beat;
    }
    void SetTempo(float tempo) override;  // slot 28: 0x5A050
    // Slot 31 at 0x59B60: the instruments' voices in use.
    int GetNumVoicesInUse() const override;
    // Slots 32-37 at 0x5AE40 through 0x5AE90: the rack has no channels of
    // its own.
    void SetMidiChannelVolume(float volume, float fadeSecs, signed char channel) override {
        static_cast<void>(volume);
        static_cast<void>(fadeSecs);
        static_cast<void>(channel);
    }
    float GetMidiChannelVolume(signed char channel) const override {
        static_cast<void>(channel);
        return 0.0F;
    }
    void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) override {
        static_cast<void>(gain);
        static_cast<void>(fadeSecs);
        static_cast<void>(channel);
    }
    float GetMidiChannelGain(signed char channel) const override {
        static_cast<void>(channel);
        return 1.0F;
    }
    void SetMidiChannelMute(bool mute, signed char channel) override {
        static_cast<void>(mute);
        static_cast<void>(channel);
    }
    bool GetMidiChannelMute(signed char channel) const override {
        static_cast<void>(channel);
        return false;
    }

    // InstrumentGenerator overrides. Slots 40 and 42-46 keep the base's
    // defaults.
    void AddAudioThreadClient(AudioBusCallable* client) override;     // slot 38: 0x58520
    void RemoveAudioThreadClient(AudioBusCallable* client) override;  // slot 39: 0x58590
    // Slot 41 at 0x5AEA0: ignored; the indexed SetPatch reaches an
    // instrument.
    void SetPatch(const ResourcePtr<FusionPatchResource>& patch, int channel) override {
        static_cast<void>(patch);
        static_cast<void>(channel);
    }
    // Slot 47 at 0x59F60: kept for instruments set later.
    void SetTranspose(int semitones) override;
    int GetTranspose() const override {  // slot 48: 0x5AEB0
        return mTranspose;
    }
    // Slot 49 at 0x59FD0: kept for instruments set later.
    void SetTimeStretchMode(int algorithm, int formantMode) override;
    // Slot 50 at 0x5AEC0: true only when both are set.
    bool GetTimeStretchMode(int* algorithm, int* formantMode) const override {
        *algorithm = mTimeStretchAlgorithm;
        *formantMode = mTimeStretchFormantMode;
        return mTimeStretchAlgorithm != 0 && mTimeStretchFormantMode != 0;
    }

    // AudioGenerator and AudioBusCallable overrides, in their primary slots
    // 51-74. The AudioGenerator vtable reaches them through this-adjusting
    // copies.
    // Slot 51 at 0x5AEE0 (copy 0x5B0B0).
    AudioGenerator* GetGeneratorOfType(Symbol type) override {
        return type == kTypeId ? this : nullptr;
    }
    // Slot 52 at 0x5AF00 (copy 0x5B0D0).
    Symbol GetTypeId() override {
        return kTypeId;
    }
    // Slot 53 at 0x5AF10 (copy 0x5B060).
    void _InitTypeId() override {
        kTypeId = Symbol("SynthRackGenerator");
    }
    // Slot 54 at 0x58420 (copy 0x584A0): clears the transpose, the
    // time-stretch mode and the bus voice and prepares the bus for stereo
    // 128-sample blocks.
    void Init(AudioGeneratorManager* manager, int index) override;
    void Pause() override;                     // slot 55: 0x58C20 (copy 0x58C50)
    void Continue() override;                  // slot 56: 0x58C80 (copy 0x58CB0)
    void Stop() override;                      // slot 57: 0x58D40 (copy 0x58D70)
    float GetElapsedMs() override;             // slot 58: 0x58CE0 (copy 0x58CF0)
    float GetTimelineMs() override;            // slot 59: 0x58D00 (copy 0x58D10)
    void SeekToMs(float ms) override;          // slot 60: 0x58D20 (copy 0x58D30)
    // Slots 61-64 at 0x5AF60 through 0x5AFC0 (copies 0x5AFE0 through
    // 0x5B040) forward to the bus voice.
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
    // Slot 65 at 0x5A0C0 (copy 0x5A160): whether the bus voice or any
    // instrument took the value; every one is offered it.
    bool SetParameter(Symbol name, float value) override;
    // Slot 66 at 0x5A200 (copy 0x5A2A0): the bus voice's value, or the
    // first instrument's that has it.
    bool GetParameter(Symbol name, float& value) override;
    // Slots 67-68 at 0x59C70 and 0x59D50 (copies 0x59CE0 and 0x59DA0): the
    // instruments' speed; the first instrument reports it, or one without
    // instruments.
    void SetSpeed(float speed, bool immediate) override;
    float GetSpeed(bool* changing) override;
    // Slot 69 at 0x58AE0 (copy 0x58B80): polls the instruments; false once
    // the bus voice finishes.
    bool Poll() override;
    // Slot 70 at 0x58E40 (copy 0x58FE0): drops the clients, releases the
    // instruments and returns the generator to its pool.
    void Release() override;
    // Slot 71 at 0x58620 (AudioBusCallable copy 0x58710): prepares each
    // audio-thread client and drops those that return false.
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;
    // Slots 72-73 at 0x59DF0 and 0x59ED0 (copies 0x59E60 and 0x59F20): the
    // instruments' play scale, reported by the first instrument or one.
    void SetPlayScale(float scale) override;
    float GetPlayScale() override;
    void Kill() override;                      // slot 74: 0x58DA0 (copy 0x58DF0)

    // Takes a bus voice for the request's render target and emitter and
    // starts it on this rack, paused or playing as requested. At 0x58370;
    // SynthRackGeneratorManager::Play inlines it. Name not in the reference
    // map.
    bool Setup(const PlayArgs& args);

    // Puts the instrument in the slot, growing the rack, and releases the
    // slot's previous instrument. The instrument takes the rack's emitter,
    // the slot's volume and mute, and the rack's transpose, time-stretch
    // mode and play scale. At 0x58FF0.
    void SetInstrument(int index, InstrumentGenerator* instrument);

    // The indexed members reach one slot's instrument and do nothing for an
    // empty or missing slot. Names not in the reference map.
    void SetInstrumentMute(int index, bool mute);  // 0x592A0
    bool GetInstrumentMute(int index) const;       // 0x59300
    // In decibels: the instrument's gain is 10^(dB/20). At 0x59350.
    void SetInstrumentVolume(int index, float volumeDb, float fadeSecs);
    // -96 for a silent instrument. At 0x593E0.
    float GetInstrumentVolume(int index) const;
    void SetInstrumentGain(int index, float gain, float fadeSecs);  // 0x59460
    float GetInstrumentGain(int index) const;                       // 0x594B0
    void NoteOn(
        int index,
        signed char note,
        signed char velocity,
        signed char channel,
        float startOffsetMs);  // 0x59500
    void NoteOff(int index, signed char note, signed char channel);  // 0x59550
    void SetPitchBend(int index, float bend, signed char channel);   // 0x595A0
    // The binary reads the slot the channel names, not the index. At
    // 0x595F0.
    float GetPitchBend(int index, signed char channel) const;
    void SetController(
        int index,
        ControllerID controller,
        signed char msb,
        signed char lsb,
        signed char channel);  // 0x59650
    // As in GetPitchBend, the channel selects the slot. At 0x596A0.
    void GetController(
        int index,
        ControllerID controller,
        signed char& msb,
        signed char& lsb,
        signed char channel) const;
    void SetInstrumentName(int index, Symbol name);  // 0x59710
    Symbol GetInstrumentName(int index) const;       // 0x59750
    // The slot with the name, or -1. At 0x597D0.
    int FindInstrumentChannel(Symbol name) const;
    InstrumentGenerator* FindInstrument(Symbol name) const;  // 0x59830
    InstrumentGenerator* GetInstrument(int index) const;     // 0x598A0
    // 0x598E0.
    void SetPatch(int index, const ResourcePtr<FusionPatchResource>& patch, int channel);
    // 0x59930 and 0x59980.
    void SetMidiChannelVolume(int index, float volume, float fadeSecs, signed char channel);
    float GetMidiChannelVolume(int index, signed char channel) const;
    // The instrument's channel 0. At 0x599D0 and 0x59A20.
    void SetMidiChannelGain(int index, float gain, float fadeSecs);
    float GetMidiChannelGain(int index) const;
    void SetMidiChannelMute(int index, bool mute, signed char channel);  // 0x59A70
    bool GetMidiChannelMute(int index, signed char channel) const;     // 0x59AC0
    void HandleMidiMessage(
        int index,
        signed char status,
        signed char data1,
        signed char data2,
        float startOffsetMs);  // 0x59B10

    static Symbol kTypeId;  // 0x19C8788

    // Field names are not in the reference map.
    eastl::vector<RackSlot> mInstruments;
    // Not initialized by the constructor; Init clears it.
    int mTranspose;
    float mBeat;
    int mTimeStretchAlgorithm;
    int mTimeStretchFormantMode;
    AudioBusGenerator* mBusGenerator;
    ClientList mAudioThreadClients;
    CritSec mClientListLock;
};

static_assert(sizeof(SynthRackGenerator::RackSlot) == 24);
static_assert(offsetof(SynthRackGenerator::RackSlot, mVolume) == 16);
static_assert(offsetof(SynthRackGenerator::RackSlot, mMute) == 20);
static_assert(offsetof(SynthRackGenerator, mManager) == 0x138 + 16);
static_assert(offsetof(SynthRackGenerator, mMakeGuard) == 0x190);
static_assert(offsetof(SynthRackGenerator, mInstruments) == 0x1B0);
static_assert(offsetof(SynthRackGenerator, mTranspose) == 0x1D0);
static_assert(offsetof(SynthRackGenerator, mBeat) == 0x1D4);
static_assert(offsetof(SynthRackGenerator, mTimeStretchFormantMode) == 0x1DC);
static_assert(offsetof(SynthRackGenerator, mBusGenerator) == 0x1E0);
static_assert(offsetof(SynthRackGenerator, mAudioThreadClients) == 0x1E8);
static_assert(offsetof(SynthRackGenerator, mClientListLock) == 0x200);
static_assert(sizeof(SynthRackGenerator) == 0x210);

// The pool of SynthRack generators. The class is newer than the reference
// map; its names follow the other managers'. The vtable is at 0x18E2598.
// The object is 72 bytes.
class SynthRackGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into InitGeneratorManager<SynthRackGeneratorManager> (0x6970).
    explicit SynthRackGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The manager's id, interned on first use. Inlined into GetId (0x5A5A0)
    // and the Fusion generator's object (0x448BB), which share its static at
    // 0x19C85A8. Name not in the reference map.
    static Symbol Id() {
        static Symbol id("");
        if (id == Symbol("")) {
            id = Symbol("SynthRackGeneratorManager");
        }
        return id;
    }
    // The resource extension, interned on first use. Inlined into
    // GetResourceExt (0x5A640); the static is at 0x19C8790. The binary
    // reports ".multi_inst", while Play answers ".synth_rack". Name not in
    // the reference map.
    static Symbol ResExt() {
        static Symbol resExt("");
        if (resExt == Symbol("")) {
            resExt = Symbol(".multi_inst");
        }
        return resExt;
    }

    // Slot 0 at 0x58140. Plays a rack for the name ".synth_rack" only.
    AudioGenerator* Play(const PlayArgs& args) override;
    void Init() override;                  // slot 3: 0x58130
    int GetIndex() override;               // slot 6: 0x5A590
    Symbol GetId() override;               // slot 7: 0x5A5A0
    Symbol GetResourceExt() override;      // slot 8: 0x5A640
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x5A6E0
    void SendStopToAllGenerators() override;  // slot 10: 0x5A770
    void SendKillToAllGenerators() override;  // slot 11: 0x5A7E0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x5A860
    void _SetManagerIndex(int index) override;      // slot 13: 0x5A9B0
    void _InitGeneratorPool() override;    // slot 14: 0x5A9C0
    bool _DeleteGeneratorPool() override;  // slot 15: 0x5ACD0
    ~SynthRackGeneratorManager() override;  // slots 16-17: 0x5AD80, 0x5AD90

    static const char* kIdStr;  // 0x19B00E0

    SynthRackGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(SynthRackGeneratorManager, mPool) == 64);
static_assert(sizeof(SynthRackGeneratorManager) == 72);
