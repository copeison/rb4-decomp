#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/fmod/playback/FmodAudioBusGenerator.h"
#include "utl/text/Symbol.h"

// A rack of instruments played as one through a bus voice of
// gAudioBusGeneratorManager (audio/SynthRackGenerator.o). The class is newer
// than the reference map, so its names are inferred, and it has not been
// reconstructed. Its layout and vtable shape match MultiFusionGenerator's:
// the primary vtable is at 0x18E21E8, the AudioGenerator vtable at
// 0x18E2450 and the AudioBusCallable vtable at 0x18E2560. The object is 528
// bytes.
class SynthRackGenerator : public InstrumentGenerator, public AudioBusCallable {
public:
    // Inlined into SynthRackGeneratorManager::_InitGeneratorPool (0x5A9C0):
    // VirtualInstrument(nullptr) at 0x66DF0, the inline AudioGenerator and
    // AudioBusCallable constructors, the base vtables 0x18E15F0 (the
    // InstrumentGenerator pair) and 0x18E0A98, then the members. No
    // out-of-line copy exists.
    SynthRackGenerator();
    ~SynthRackGenerator() override;  // slots 0-1: 0x587F0, 0x58A60

    // AudioBus and VirtualInstrument overrides, in the primary slots.
    bool Process(AudioBuffer<float>& buffer) override;  // slot 4: 0x5A340
    // Slot 10 at 0x5ADB0, also AudioGenerator's slot 17 (0x43FC0).
    bool IsInstrument() override;
    void ResetInstrumentState() override;  // slot 13: 0x59170
    void ResetMidiState() override;        // slot 14: 0x59210
    void NoteOn(signed char note, signed char velocity, signed char channel, float startOffsetMs) override;  // slot 15: 0x5ADC0
    bool IsNotePlaying(signed char note) override;                    // slot 16: 0x5ADD0
    void NoteOff(signed char note, signed char channel) override;     // slot 17: 0x5ADE0
    void SetPitchBend(float bend, signed char channel) override;       // slot 19: 0x5ADF0
    float GetPitchBend(signed char channel) const override;           // slot 20: 0x5AE00
    void SetController(ControllerID controller, signed char msb, signed char lsb, signed char channel) override;  // slot 21: 0x5AE10
    void GetController(
        ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const override;  // slot 22: 0x5AE20
    using VirtualInstrument::GetController;
    using VirtualInstrument::SetController;
    void KillAllVoices() override;          // slot 25: 0x59BD0
    void AllNotesOff() override;            // slot 26: 0x59C20
    void SetBeat(float beat) override;      // slot 27: 0x5AE30
    void SetTempo(float tempo) override;    // slot 28: 0x5A050
    int GetNumVoicesInUse() const override;  // slot 31: 0x59B60
    void SetMidiChannelVolume(float volume, float fadeSecs, signed char channel) override;  // slot 32: 0x5AE40
    float GetMidiChannelVolume(signed char channel) const override;                       // slot 33: 0x5AE50
    void SetMidiChannelGain(float gain, float fadeSecs, signed char channel) override;    // slot 34: 0x5AE60
    float GetMidiChannelGain(signed char channel) const override;                         // slot 35: 0x5AE70
    void SetMidiChannelMute(bool mute, signed char channel) override;                     // slot 36: 0x5AE80
    bool GetMidiChannelMute(signed char channel) const override;                          // slot 37: 0x5AE90

    // InstrumentGenerator overrides. Slots 40 and 42-46 keep the base's
    // entries in the binary.
    void AddAudioThreadClient(AudioBusCallable* client) override;     // slot 38: 0x58520
    void RemoveAudioThreadClient(AudioBusCallable* client) override;  // slot 39: 0x58590
    bool SupportsAudioThreadClients() const override;
    void SetPatch(const ResourcePtr<FusionPatchResource>& patch) override;  // slot 41: 0x5AEA0
    bool AddSlave(unsigned int handle, InstrumentSlaveType type) override;
    bool RemoveSlave(unsigned int handle) override;
    void ClearGeneratorFlag() override;
    void SetGeneratorFlag(int flag) override;
    bool HasGeneratorFlag() const override;
    void SetTranspose(int semitones) override;                        // slot 47: 0x59F60
    int GetTranspose() const override;                                // slot 48: 0x5AEB0
    void SetTimeStretchMode(int algorithm, int formantMode) override;  // slot 49: 0x59FD0
    bool GetTimeStretchMode(int* algorithm, int* formantMode) const override;  // slot 50: 0x5AEC0

    // AudioGenerator and AudioBusCallable overrides. The binary's primary
    // vtable appends 24 entries for them (slots 51-74) in an order this
    // declaration does not reproduce, so callers reach them through the
    // declaring base. The addresses are the AudioGenerator vtable's
    // this-adjusting entries, or the bodies its thunks jump to; most forward
    // to the bus voice.
    void Pause() override;                                    // AudioGenerator slot 0: 0x58C50
    void Continue() override;                                 // AudioGenerator slot 1: 0x58CB0
    void Stop() override;                                     // AudioGenerator slot 2: 0x58D70
    float GetElapsedMs() override;                            // AudioGenerator slot 4: 0x58CF0
    float GetTimelineMs() override;                           // AudioGenerator slot 5: 0x58D10
    void SeekToMs(float ms) override;                         // AudioGenerator slot 7: 0x58D30
    void SetSpeed(float speed, bool immediate) override;      // AudioGenerator slot 8: 0x59CE0
    float GetSpeed(bool* changing) override;                  // AudioGenerator slot 9: 0x59DA0
    bool SetParameter(Symbol name, float value) override;     // AudioGenerator slot 10: 0x5A160
    bool GetParameter(Symbol name, float& value) override;    // AudioGenerator slot 11: 0x5A2A0
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // AudioGenerator slot 12: 0x5AFE0
    float GetGain() const override;                           // AudioGenerator slot 13: 0x5B000
    void SetMute(bool mute, bool immediate) override;         // AudioGenerator slot 14: 0x5B020
    bool GetMute() const override;                            // AudioGenerator slot 15: 0x5B040
    // AudioGenerator slot 19 at 0x584A0, a this-adjusting copy of the
    // primary entry at 0x58420.
    void Init(AudioGeneratorManager* manager, int index) override;
    bool Poll() override;                                     // AudioGenerator slot 22: 0x58B80
    void Release() override;                                  // AudioGenerator slot 23: 0x58E40
    void SetPlayScale(float scale) override;                  // AudioGenerator slot 25: 0x59E60
    float GetPlayScale() override;                            // AudioGenerator slot 26: 0x59F20
    void _InitTypeId() override;                              // AudioGenerator slot 28: 0x5B060
    void Kill() override;                                     // AudioGenerator slot 29: 0x58DF0
    AudioGenerator* GetGeneratorOfType(Symbol type) override; // AudioGenerator slot 30: 0x5B0B0
    Symbol GetTypeId() override;                              // AudioGenerator slot 31: 0x5B0D0
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // AudioBusCallable slot 2: 0x58710

    // Takes a bus voice for the request's render target and emitter and
    // starts it on this rack, paused or playing as requested. At 0x58370;
    // SynthRackGeneratorManager::Play inlines it. Name not in the reference
    // map.
    bool Setup(const PlayArgs& args);

    static Symbol kTypeId;  // 0x19C8788

    // Field names are not in the reference map.
    // Not reconstructed: a vector of the rack's instruments, which
    // SetTranspose and SetTimeStretchMode forward to.
    unsigned char mInstrumentState[32];
    int mTranspose;
    float mBeat;
    int mTimeStretchAlgorithm;
    int mTimeStretchFormantMode;
    AudioBusGenerator* mBusGenerator;
    // Not reconstructed: the audio thread clients (+488) with their lock
    // (+512).
    unsigned char mClientState[40];
};

static_assert(offsetof(SynthRackGenerator, mManager) == 0x138 + 16);
static_assert(offsetof(SynthRackGenerator, mMakeGuard) == 0x190);
static_assert(offsetof(SynthRackGenerator, mInstrumentState) == 0x1B0);
static_assert(offsetof(SynthRackGenerator, mTranspose) == 0x1D0);
static_assert(offsetof(SynthRackGenerator, mBeat) == 0x1D4);
static_assert(offsetof(SynthRackGenerator, mTimeStretchFormantMode) == 0x1DC);
static_assert(offsetof(SynthRackGenerator, mBusGenerator) == 0x1E0);
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
