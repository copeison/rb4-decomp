#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/music/MusicGenerator.h"
#include "audio/core/output/AudioBus.h"

// A silent music generator that only advances a song timeline
// (audio/MusicTimelineGenerator.o). Only the members its manager uses are
// declared; the rest of the class has not been reconstructed. The primary
// vtable is at 0x18E1CC8 and the AudioBusCallable one at 0x18E2110
// (+0x158). The object is 768 bytes.
class MusicTimelineGenerator : public MusicGenerator, public AudioBusCallable {
public:
    // Inlined into MusicTimelineGeneratorManager::_InitGeneratorPool at
    // 0x57B50; the map has it out of line. It runs the MusicGenerator and
    // AudioBusCallable constructors and builds the members from +0x188 on,
    // among them a CritSec at +0x2C8 and a list at +0x2E8.
    MusicTimelineGenerator();

    float GetElapsedMs() override;               // slot 4: 0x57020
    float GetTimelineMs() override;              // slot 5: 0x57030
    void SeekToMs(float ms) override;            // slot 7: 0x57040
    bool SetParameter(Symbol name, float value) override;   // slot 10: 0x57F40
    bool GetParameter(Symbol name, float& value) override;  // slot 11: 0x57F50
    ~MusicTimelineGenerator() override;          // slots 20-21: 0x56C90, 0x56E30
    bool Poll() override;                        // slot 22: 0x56FB0
    void Release() override;                     // slot 23: 0x574C0
    void _InitTypeId() override;                 // slot 28: 0x57F60
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x57FB0
    Symbol GetTypeId() override;                 // slot 31: 0x57FD0
    bool IsReady() override;                     // slot 32: 0x57FE0
    // The AudioBusCallable slot 2; its thunk is at 0x574A0.
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;

    // Starts the timeline for the request. At 0x56880.
    void Setup(const PlayArgs& args);

    static Symbol kTypeId;  // 0x19C8758

    // The generator's own state after the AudioBusCallable base; not
    // reconstructed.
    unsigned char mTimelineState[0x300 - 0x180];
};

static_assert(sizeof(MusicTimelineGenerator) == 0x300);

// The pool of MusicTimeline generators. It plays only the sound named by
// its resource extension. The vtable is at 0x18E2148. The object is 72
// bytes.
class MusicTimelineGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into
    // SoundManager::InitGeneratorManager<MusicTimelineGeneratorManager>.
    // Name not in the reference map.
    explicit MusicTimelineGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The manager's id and resource extension, interned on first use and
    // inlined into GetId, GetResourceExt and Play. The local statics are at
    // 0x19C8770 and 0x19C8760.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("MusicTimelineGeneratorManager");
        }
        return id;
    }
    static Symbol ResExt() {
        static Symbol resExt;
        if (resExt == Symbol()) {
            resExt = Symbol(".musictimeline");
        }
        return resExt;
    }

    // Slot 3 is AudioGeneratorManager::Init (0x40500).
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x566F0
    int GetIndex() override;               // slot 6: 0x57740
    Symbol GetId() override;               // slot 7: 0x57750
    Symbol GetResourceExt() override;      // slot 8: 0x577F0
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x57890
    void SendStopToAllGenerators() override;  // slot 10: 0x57910
    void SendKillToAllGenerators() override;  // slot 11: 0x57980
    void GetActiveHandles(void* handles) override;  // slot 12: 0x579F0
    void _SetManagerIndex(int index) override;      // slot 13: 0x57B40
    void _InitGeneratorPool() override;    // slot 14: 0x57B50
    bool _DeleteGeneratorPool() override;  // slot 15: 0x57E60
    ~MusicTimelineGeneratorManager() override;  // slots 16-17: 0x57F10, 0x57F20

    // Both unreferenced in this build; Play compares with ResExt().
    static const char* kMusicTimelineSoundId;  // 0x19B00D0
    static const char* kIdStr;                 // 0x19B00D8

    MusicTimelineGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(MusicTimelineGeneratorManager, mPool) == 64);
static_assert(sizeof(MusicTimelineGeneratorManager) == 72);
