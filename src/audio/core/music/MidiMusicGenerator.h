#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/music/MidiPlayCursor.h"
#include "audio/core/music/MusicGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "entity/resources/Resource.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"

class InstrumentGenerator;
class MidiMusicResource;

// Plays a MidiMusicResource through a SynthRack instrument
// (audio/MidiMusicGenerator.o). Only the members its manager uses are
// declared; the rest of the class has not been reconstructed. The primary
// vtable is at 0x18E04E0, the MidiPlayCursor one at 0x18E0950 (+0x158) and
// the AudioBusCallable one at 0x18E09C0 (+0x3B0). The object is 1896 bytes.
class MidiMusicGenerator : public MusicGenerator, public MidiPlayCursor, public AudioBusCallable {
public:
    // Inlined into MidiMusicGeneratorManager::_InitGeneratorPool at
    // 0x467C0; the map has it out of line. It runs the MusicGenerator and
    // MidiPlayCursor constructors and the AudioBusCallable one, clears the
    // pointers at +0x3E0, +0x538 and +0x750 and constructs the member at
    // +0x3E8 (0xABED0).
    MidiMusicGenerator();

    float GetElapsedMs() override;               // slot 4: 0x45940
    float GetTimelineMs() override;              // slot 5: 0x45950
    void SeekToMs(float ms) override;            // slot 7: 0x45970
    bool SetParameter(Symbol name, float value) override;   // slot 10: 0x46B10
    bool GetParameter(Symbol name, float& value) override;  // slot 11: 0x46B30
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x46B50
    float GetGain() const override;              // slot 13: 0x46B70
    // The map has SetMute(bool). At 0x46B90.
    void SetMute(bool mute, bool immediate) override;  // slot 14
    bool GetMute() const override;               // slot 15: 0x46BB0
    ~MidiMusicGenerator() override;              // slots 20-21: 0x45060, 0x45180
    bool Poll() override;                        // slot 22: 0x45470
    void Release() override;                     // slot 23: 0x461B0
    void SetPlayScale(float scale) override;     // slot 25: 0x46BE0
    float GetPlayScale() override;               // slot 26: 0x46C10
    void _InitTypeId() override;                 // slot 28: 0x46C40
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x46C90
    Symbol GetTypeId() override;                 // slot 31: 0x46CD0
    bool IsReady() override;                     // slot 32: 0x46CE0
    // The AudioBusCallable slot 2; its thunk is at 0x45FE0.
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;

    // Binds the song and the SynthRack instrument that plays it. At 0x44AA0.
    // The map has Setup(MidiMusicResource*, MultiInstrumentGenerator*,
    // PlayArgs const&).
    void Setup(const ResourcePtr<MidiMusicResource>& resource, InstrumentGenerator* instrument,
        const PlayArgs& args);

    static Symbol kTypeId;  // 0x19C8538

    // The generator's own state after the AudioBusCallable base; not
    // reconstructed.
    unsigned char mMidiMusicState[0x768 - 0x3D8];
};

static_assert(sizeof(MidiMusicGenerator) == 0x768);

// The pool of MidiMusic generators. Its resources register by sound name.
// The vtable is at 0x18E09F8. The object is 72 bytes.
class MidiMusicGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into SoundManager::InitGeneratorManager<MidiMusicGeneratorManager>.
    // Name not in the reference map.
    explicit MidiMusicGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The manager's id and resource extension, interned on first use and
    // inlined into GetId and GetResourceExt. The local statics are at
    // 0x19C8588 and 0x19C8598.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("MidiMusicGeneratorManager");
        }
        return id;
    }
    static Symbol ResExt() {
        static Symbol resExt;
        if (resExt == Symbol()) {
            resExt = Symbol(".midisong");
        }
        return resExt;
    }

    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x44700
    void Init() override;                  // slot 3: 0x44430
    int GetIndex() override;               // slot 6: 0x463C0
    Symbol GetId() override;               // slot 7: 0x463D0
    Symbol GetResourceExt() override;      // slot 8: 0x46470
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x46510
    void SendStopToAllGenerators() override;  // slot 10: 0x46580
    void SendKillToAllGenerators() override;  // slot 11: 0x465F0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x46660
    void _SetManagerIndex(int index) override;      // slot 13: 0x467B0
    void _InitGeneratorPool() override;    // slot 14: 0x467C0
    bool _DeleteGeneratorPool() override;  // slot 15: 0x46990
    ~MidiMusicGeneratorManager() override;  // slots 16-17: 0x46A40, 0x46A50

    // Adds a loaded resource under its sound name. At 0x44440.
    static void RegisterMidiMusicResource(MidiMusicResource* resource);
    // Removes every entry of the resource; false when there was none. At
    // 0x44530.
    static bool UnregisterMidiMusicResource(MidiMusicResource* resource);
    // Looks up a registered resource by sound name. At 0x44600, with no
    // caller in this build; Play inlines its own lookup. Name not in the
    // reference map.
    static ResourcePtr<MidiMusicResource> FindMidiMusicResource(Symbol name);

    // Unreferenced in this build.
    static const char* kIdStr;  // 0x19B00A8
    // The registered resources by sound name.
    static eastl::map<Symbol, MidiMusicResource*> mMidiMusicMap;  // 0x19C8550
    // Guards mMidiMusicMap. Name not in the reference map.
    static CritSec mMidiMusicMapLock;  // 0x19C8540

    MidiMusicGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(MidiMusicGeneratorManager, mPool) == 64);
static_assert(sizeof(MidiMusicGeneratorManager) == 72);
