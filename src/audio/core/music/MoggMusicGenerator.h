#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/music/MusicGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "entity/resources/Resource.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"

class MoggGenerator;
class MoggMusicResource;

// Plays a MoggMusicResource through a voice of the Mogg generator manager
// (audio/MoggMusicGenerator.o). Only the members its manager uses are
// declared; the rest of the class has not been reconstructed. The primary
// vtable is at 0x18E0C80 and the AudioBusCallable one at 0x18E10C8
// (+0x158). The object is 768 bytes.
class MoggMusicGenerator : public MusicGenerator, public AudioBusCallable {
public:
    // Inlined into MoggMusicGeneratorManager::_InitGeneratorPool at
    // 0x4DB60; the map has it out of line. It runs the MusicGenerator
    // and AudioBusCallable constructors, clears mMoggGenerator and
    // mResource, initializes the list at +0x198 and constructs the member at
    // +0x1B0 (0xABED0).
    MoggMusicGenerator();

    float GetElapsedMs() override;               // slot 4: 0x4DDF0
    float GetTimelineMs() override;              // slot 5: 0x4DE00
    float GetLengthMs() const override;          // slot 6: 0x4DE10
    void SeekToMs(float ms) override;            // slot 7: 0x4DE20
    float GetSpeed(bool* changing) override;     // slot 9: 0x4DE30
    bool SetParameter(Symbol name, float value) override;   // slot 10: 0x4DE60
    bool GetParameter(Symbol name, float& value) override;  // slot 11: 0x4DE70
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x4DE80
    float GetGain() const override;              // slot 13: 0x4DE90
    // The map has SetMute(bool). At 0x4DEA0.
    void SetMute(bool mute, bool immediate) override;  // slot 14
    bool GetMute() const override;               // slot 15: 0x4DEC0
    ~MoggMusicGenerator() override;              // slots 20-21: 0x4C510, 0x4C660
    bool Poll() override;                        // slot 22: 0x4D3E0
    void Release() override;                     // slot 23: 0x4D450
    float GetPrimaryStreamValue() override;      // slot 27: 0x4DED0
    void _InitTypeId() override;                 // slot 28: 0x4DEF0
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x4DF40
    Symbol GetTypeId() override;                 // slot 31: 0x4DF70
    // Slot 32 at 0x4DF80: whether the Mogg voice is ready.
    bool IsReady() override;
    // The AudioBusCallable slot 2; its thunk is at 0x4D3C0.
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;

    // Binds the song and the Mogg voice that streams it. At 0x4C390. The
    // map has Setup(MoggGenerator*, MoggMusicResource*, PlayArgs const&).
    void Setup(MoggGenerator* mogg, const ResourcePtr<MoggMusicResource>& resource,
        const PlayArgs& args);

    static Symbol kTypeId;  // 0x19C8648

    // Field names are not in the reference map.
    // The Mogg voice that streams the song; the generator forwards most
    // queries to it.
    MoggGenerator* mMoggGenerator;
    ResourcePtr<MoggMusicResource> mResource;
    // The rest of the object; not reconstructed.
    unsigned char mMoggMusicState[0x300 - 0x190];
};

static_assert(offsetof(MoggMusicGenerator, mMoggGenerator) == 0x180);
static_assert(offsetof(MoggMusicGenerator, mResource) == 0x188);
static_assert(sizeof(MoggMusicGenerator) == 0x300);

// The pool of MoggMusic generators. Its resources register by sound name.
// The vtable is at 0x18E1100. The object is 72 bytes.
class MoggMusicGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into SoundManager::InitGeneratorManager<MoggMusicGeneratorManager>.
    // Name not in the reference map.
    explicit MoggMusicGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The manager's id and resource extension, interned on first use and
    // inlined into GetId and GetResourceExt. The local statics are at
    // 0x19C86A8 and 0x19C86B8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("MoggMusicGeneratorManager");
        }
        return id;
    }
    static Symbol ResExt() {
        static Symbol resExt;
        if (resExt == Symbol()) {
            resExt = Symbol(".moggsong");
        }
        return resExt;
    }

    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x4BF00
    void Init() override;                  // slot 3: 0x4BD30
    int GetIndex() override;               // slot 6: 0x4D750
    Symbol GetId() override;               // slot 7: 0x4D760
    Symbol GetResourceExt() override;      // slot 8: 0x4D800
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x4D8A0
    void SendStopToAllGenerators() override;  // slot 10: 0x4D920
    void SendKillToAllGenerators() override;  // slot 11: 0x4D990
    void GetActiveHandles(void* handles) override;  // slot 12: 0x4DA00
    void _SetManagerIndex(int index) override;      // slot 13: 0x4DB50
    void _InitGeneratorPool() override;    // slot 14: 0x4DB60
    bool _DeleteGeneratorPool() override;  // slot 15: 0x4DD10
    ~MoggMusicGeneratorManager() override;  // slots 16-17: 0x4DDC0, 0x4DDD0

    // Adds a loaded resource under its sound name. At 0x4BD40.
    static void RegisterMoggMusicResource(MoggMusicResource* resource);
    // Removes every entry of the resource; false when there was none. At
    // 0x4BE30.
    static bool UnregisterMoggMusicResource(MoggMusicResource* resource);

    // Unreferenced in this build.
    static const char* kIdStr;  // 0x19B00C0
    // The registered resources by sound name.
    static eastl::map<Symbol, MoggMusicResource*> mMoggMusicMap;  // 0x19C8660
    // Guards mMoggMusicMap. Name not in the reference map.
    static CritSec mMoggMusicMapLock;  // 0x19C8650

    MoggMusicGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(MoggMusicGeneratorManager, mPool) == 64);
static_assert(sizeof(MoggMusicGeneratorManager) == 72);
