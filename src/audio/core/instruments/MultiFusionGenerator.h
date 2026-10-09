#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/fmod/playback/FmodAudioBusGenerator.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"
#include "utl/text/Symbol.h"

class MultiFusionResource;

// An instrument that routes each of 16 MIDI channels to its own instrument
// generator and plays through a bus voice of gAudioBusGeneratorManager
// (audio/MultiInstrumentGenerator.o, the map's MultiInstrumentGenerator).
// The class has not been reconstructed. Its primary vtable is at 0x18E11A0,
// the AudioGenerator vtable at 0x18E1408 and the AudioBusCallable vtable at
// 0x18E1518. The object is 840 bytes.
class MultiFusionGenerator : public InstrumentGenerator, public AudioBusCallable {
public:
    // Inlined into MultiFusionGeneratorManager::_InitGeneratorPool (0x51930):
    // VirtualInstrument(nullptr) at 0x66DF0, the inline AudioGenerator and
    // AudioBusCallable constructors, the base vtables 0x18E15F0 (the
    // InstrumentGenerator pair) and 0x18E0A98, then the members. No
    // out-of-line copy exists.
    MultiFusionGenerator();
    ~MultiFusionGenerator() override;  // slots 0-1: 0x4F790, 0x4FA00

    // AudioBus and VirtualInstrument overrides, in the primary slots.
    bool Process(AudioBuffer<float>& buffer) override;  // slot 4: 0x512C0
    // Slot 10 at 0x51D60, also AudioGenerator's slot 17 (0x43FC0).
    bool IsInstrument() override;
    void ResetInstrumentState() override;  // slot 13: 0x501E0
    void ResetMidiState() override;        // slot 14: 0x50260
    void NoteOn(signed char note, signed char velocity, signed char channel, float startOffsetMs) override;  // slot 15: 0x51D90
    bool IsNotePlaying(signed char note) override;                    // slot 16: 0x51DC0
    void NoteOff(signed char note, signed char channel) override;     // slot 17: 0x51DF0
    void SetExtraPitchBend(float bend, signed char channel) override;  // slot 18: 0x503C0
    void SetPitchBend(float bend, signed char channel) override;       // slot 19: 0x51E20
    float GetPitchBend(signed char channel) const override;           // slot 20: 0x51E50
    void SetController(ControllerID controller, signed char msb, signed char lsb, signed char channel) override;  // slot 21: 0x51E80
    void GetController(
        ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const override;  // slot 22: 0x51EB0
    using VirtualInstrument::GetController;
    using VirtualInstrument::SetController;
    void KillAllVoices() override;          // slot 25: 0x50750
    void AllNotesOff() override;            // slot 26: 0x508C0
    void SetTempo(float tempo) override;    // slot 28: 0x51050
    int GetNumVoicesInUse() const override;  // slot 31: 0x50700
    void SetMidiChannelVolume(float volume, float fadeSecs, signed char channel) override;  // slot 32: 0x50560
    float GetMidiChannelVolume(signed char channel) const override;                       // slot 33: 0x505E0
    void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) override;    // slot 34: 0x505F0
    float GetMidiChannelGain(signed char channel) const override;                         // slot 35: 0x50670
    void SetMidiChannelMute(bool mute, signed char channel) override;                     // slot 36: 0x506A0
    bool GetMidiChannelMute(signed char channel) const override;                          // slot 37: 0x506F0

    // InstrumentGenerator overrides. Slots 40 and 42-46 keep the base's
    // entries in the binary.
    void AddAudioThreadClient(AudioBusCallable* client) override;     // slot 38: 0x4F4B0
    void RemoveAudioThreadClient(AudioBusCallable* client) override;  // slot 39: 0x4F520
    bool SupportsAudioThreadClients() const override;
    void SetPatch(const ResourcePtr<FusionPatchResource>& patch) override;  // slot 41: 0x51F00
    bool AddSlave(unsigned int handle, InstrumentSlaveType type) override;
    bool RemoveSlave(unsigned int handle) override;
    void ClearGeneratorFlag() override;
    void SetGeneratorFlag(int flag) override;
    bool HasGeneratorFlag() const override;
    void SetTranspose(int semitones) override;                        // slot 47: 0x50FA0
    int GetTranspose() const override;                                // slot 48: 0x51F70
    void SetTimeStretchMode(int algorithm, int formantMode) override;  // slot 49: 0x50FF0
    bool GetTimeStretchMode(int* algorithm, int* formantMode) const override;  // slot 50: 0x51F80

    // AudioGenerator and AudioBusCallable overrides. The binary's primary
    // vtable appends 24 entries for them (slots 51-74) in an order this
    // declaration does not reproduce, so callers reach them through the
    // declaring base. The addresses are the AudioGenerator vtable's
    // this-adjusting entries, or the bodies its thunks jump to; most forward
    // to the bus voice.
    void Pause() override;                                    // AudioGenerator slot 0: 0x4FD00
    void Continue() override;                                 // AudioGenerator slot 1: 0x4FD60
    void Stop() override;                                     // AudioGenerator slot 2: 0x4FE20
    float GetElapsedMs() override;                            // AudioGenerator slot 4: 0x4FDA0
    float GetTimelineMs() override;                           // AudioGenerator slot 5: 0x4FDC0
    void SeekToMs(float ms) override;                         // AudioGenerator slot 7: 0x4FDE0
    void SetSpeed(float speed, bool immediate) override;      // AudioGenerator slot 8: 0x50A90
    float GetSpeed(bool* changing) override;                  // AudioGenerator slot 9: 0x50BF0
    bool SetParameter(Symbol name, float value) override;     // AudioGenerator slot 10: 0x51130
    bool GetParameter(Symbol name, float& value) override;    // AudioGenerator slot 11: 0x51240
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // AudioGenerator slot 12: 0x520A0
    float GetGain() const override;                           // AudioGenerator slot 13: 0x520C0
    void SetMute(bool mute, bool immediate) override;         // AudioGenerator slot 14: 0x520E0
    bool GetMute() const override;                            // AudioGenerator slot 15: 0x52100
    void Init(AudioGeneratorManager* manager, int index) override;  // AudioGenerator slot 19: 0x4EE40
    bool Poll() override;                                     // AudioGenerator slot 22: 0x4FA80
    void Release() override;                                  // AudioGenerator slot 23: 0x4FEB0
    void SetPlayScale(float scale) override;                  // AudioGenerator slot 25: 0x50D50
    float GetPlayScale() override;                            // AudioGenerator slot 26: 0x50EB0
    void _InitTypeId() override;                              // AudioGenerator slot 28: 0x52120
    void Kill() override;                                     // AudioGenerator slot 29: 0x4FE80
    AudioGenerator* GetGeneratorOfType(Symbol type) override; // AudioGenerator slot 30: 0x52170
    Symbol GetTypeId() override;                              // AudioGenerator slot 31: 0x52190
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // AudioBusCallable slot 2: 0x4F6A0

    // Takes a bus voice for the render target and emitter and starts it on
    // this instrument, paused or playing as requested. Inlined into
    // MultiFusionGeneratorManager::Play (0x4E970) and Setup (0x4EC90). Name
    // not in the reference map; the map's Setup(PlayArgs const&, bool) is
    // taken to be this function and its overload.
    bool Setup(const PlayArgs& args) {
        mTranspose = 0;
        mTimeStretchAlgorithm = 0;
        mTimeStretchFormantMode = 0;
        if (gAudioBusGeneratorManager == nullptr) {
            return false;
        }
        mBusGenerator = gAudioBusGeneratorManager->_GetGenerator(mRenderTarget, args.mEmitter);
        if (mBusGenerator == nullptr) {
            return false;
        }
        SetSampleRate(static_cast<float>(mRenderTarget->mSampleRate));
        mBusGenerator->Setup(this, args, this);
        mState = static_cast<State>(kStatePlaying + args.mStartPaused);
        return true;
    }
    // Setup, then loads the channel assignments of the resource. At 0x4EC90.
    bool Setup(const PlayArgs& args, MultiFusionResource* resource);

    static Symbol kTypeId;  // 0x19C8728, "MultiFusionGenerator"

    // Field names are not in the reference map.
    // Not reconstructed: the instrument generator of each channel (+432,
    // SetTimeStretchMode forwards to them), the instrument names that
    // FindInstrumentChannel compares (+560), and a float (+688) and a flag
    // (+752) per channel.
    unsigned char mChannelState[336];
    int mTranspose;
    int mTimeStretchAlgorithm;
    int mTimeStretchFormantMode;
    AudioBusGenerator* mBusGenerator;
    // Not reconstructed: the audio thread clients (+792) with their lock
    // (+816) and the resource (+832).
    unsigned char mClientState[48];
};

static_assert(offsetof(MultiFusionGenerator, mManager) == 0x138 + 16);
static_assert(offsetof(MultiFusionGenerator, mMakeGuard) == 0x190);
static_assert(offsetof(MultiFusionGenerator, mChannelState) == 0x1B0);
static_assert(offsetof(MultiFusionGenerator, mTranspose) == 0x300);
static_assert(offsetof(MultiFusionGenerator, mTimeStretchFormantMode) == 0x308);
static_assert(offsetof(MultiFusionGenerator, mBusGenerator) == 0x310);
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
