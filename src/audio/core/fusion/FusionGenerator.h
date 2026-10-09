#pragma once

#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/fusion/FusionSampler.h"
#include "audio/core/generators/AudioBusGenerator.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/resources/FusionPatchResource.h"
#include "audio/core/system/SoundManager.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"

class AudioEmitter;
class AudioRenderTarget;

// Request for a Fusion generator, marked by PlayArgs::mFormat 2. A request
// with a slave type plays as a slave of the master instrument instead of
// through its own bus generator. Its builders, such as the audio component
// at 0x29410, inline the constructor. Names not in the reference map.
struct FusionPlayArgs : public PlayArgs {
    static constexpr int kFusionFormat = 2;

    FusionPlayArgs() : mSlaveType(static_cast<InstrumentSlaveType>(0)), mMasterHandle(0) {
        mFormat = kFusionFormat;
    }

    InstrumentSlaveType mSlaveType;  // Zero plays the patch on its own.
    unsigned int mMasterHandle;      // The instrument the slave follows.
};

static_assert(offsetof(FusionPlayArgs, mSlaveType) == 104);
static_assert(offsetof(FusionPlayArgs, mMasterHandle) == 108);
static_assert(sizeof(FusionPlayArgs) == 112);

// Retains the instrument generator a handle names, through
// SoundManager::LockIfOwned. A generator that is not an instrument is
// released at once. The map emits the constructor and destructor out of line
// in this object; this build inlines them.
class InstrumentHandleLock {
public:
    explicit InstrumentHandleLock(unsigned int handle) : mInstrument(nullptr) {
        AudioGenerator* const generator = theSoundManager.LockIfOwned(handle);
        if (generator != nullptr) {
            if (generator->IsInstrument()) {
                mInstrument = static_cast<InstrumentGenerator*>(generator);
            } else {
                --generator->mRefCount;
            }
        }
    }
    ~InstrumentHandleLock() {
        if (mInstrument != nullptr) {
            --static_cast<AudioGenerator*>(mInstrument)->mRefCount;
        }
    }

    InstrumentHandleLock(const InstrumentHandleLock&) = delete;
    InstrumentHandleLock& operator=(const InstrumentHandleLock&) = delete;

    InstrumentGenerator* operator->() const {
        return mInstrument;
    }

    InstrumentGenerator* mInstrument;  // Name not in the reference map.
};

// The playable sampler (audio/FusionGenerator.o, 0x41F80 to 0x44430). A
// generator either renders through an AudioBusGenerator that the bus
// generator manager provides, or, as a slave, is mixed by its master
// instrument. Clients added with AddAudioThreadClient are prepared before
// each block. The primary vtable is at 0x18E00C8 and the AudioGenerator
// vtable at 0x18E0330; the object is 0x5498 bytes.
class FusionGenerator : public FusionSampler {
public:
    using ClientList =
        LinkedListSizeTracked::List<AudioBusCallable, &AudioBusCallable::_mCallbackNode>;

    // Inlined into FusionGeneratorManager::_InitGeneratorPool at 0x438B0.
    // The master handle and the bus generator are set by Init.
    FusionGenerator() {}
    ~FusionGenerator() override;  // slots 0-1: 0x42AC0, 0x42C30

    // Slot 11 at 0x429C0: prepares each client and drops those that return
    // false.
    void CallPreProcessCallbacks(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;
    void AddAudioThreadClient(AudioBusCallable* client) override;     // slot 38: 0x42840
    void RemoveAudioThreadClient(AudioBusCallable* client) override;  // slot 39: 0x428B0
    // Slot 41 at 0x43D30.
    void SetPatch(const ResourcePtr<FusionPatchResource>& patch, int channel) override {
        static_cast<void>(channel);
        LoadPatch(patch);
    }
    // Slot 44 at 0x43D40: forgets the master's handle.
    void DetachedFromMaster() override {
        mMasterHandle = 0;
    }
    // Slots 45-46 at 0x43D50 and 0x43D60 store and test the master's handle.
    void AttachedToMaster(unsigned int masterHandle) override {
        mMasterHandle = masterHandle;
    }
    bool IsAttachedToMaster() const override {
        return mMasterHandle != 0;
    }

    // Slots 55-74 are the AudioGenerator overrides, which also take entries
    // in the primary vtable; the AudioGenerator vtable reaches them through
    // thunks.
    // Slot 55 at 0x43E00 (thunk 0x44060).
    AudioGenerator* GetGeneratorOfType(Symbol type) override {
        return type == kTypeId ? this : nullptr;
    }
    Symbol GetTypeId() override;  // slot 56: 0x43E20 (thunk 0x44080)
    // Slot 57 at 0x43E30 (thunk 0x44010).
    void _InitTypeId() override {
        kTypeId = Symbol("FusionGenerator");
    }
    void Pause() override;               // slot 58: 0x42E50 (thunk 0x42E80)
    void Continue() override;            // slot 59: 0x42EB0 (thunk 0x42EE0)
    // Kills the voices. A generator with a bus generator stops it; a slave
    // leaves its master and stops at once. At 0x42FF0 (thunk 0x430B0).
    void Stop() override;                // slot 60
    float GetElapsedMs() override;       // slot 61: 0x42F10 (thunk 0x42F20)
    float GetTimelineMs() override;      // slot 62: 0x42F30 (thunk 0x42F40)
    void SeekToMs(float ms) override;    // slot 63: 0x42F50 (thunk 0x42F60)
    // Slots 64-67 at 0x43E80 through 0x43EE0 (thunks 0x43F40 through
    // 0x43FA0) forward to the bus generator.
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override {
        if (mBusGenerator != nullptr) {
            mBusGenerator->SetGain(gain, fadeSecs, option);
        }
    }
    float GetGain() const override {
        return mBusGenerator != nullptr ? mBusGenerator->GetGain() : 0.0F;
    }
    // The map has SetMute(bool).
    void SetMute(bool mute, bool immediate) override {
        if (mBusGenerator != nullptr) {
            mBusGenerator->SetMute(mute, immediate);
        }
    }
    bool GetMute() const override {
        return mBusGenerator != nullptr ? mBusGenerator->GetMute() : false;
    }
    bool SetParameter(Symbol name, float value) override;   // slot 68: 0x42F70 (thunk 0x42F90)
    bool GetParameter(Symbol name, float& value) override;  // slot 69: 0x42FB0 (thunk 0x42FD0)
    // Clears the master handle and the bus generator and prepares the bus
    // for stereo 128-sample blocks. At 0x42740 (thunk 0x427C0).
    void Init(AudioGeneratorManager* manager, int index) override;  // slot 70
    // False once stopped, or once the bus generator finishes. At 0x42DB0
    // (thunk 0x42E00).
    bool Poll() override;                // slot 71
    // Releases the bus generator, the clients, the patch and the slaves,
    // then returns the generator to its pool. At 0x432E0 (thunk 0x43470).
    void Release() override;             // slot 72
    // Slot 73 at 0x43F00 (thunk 0x43FD0): the bus generator's. Name not in
    // the reference map.
    void* GetPluginData(const char* name) override;
    // Like Stop, but kills the bus generator and stops at once. At 0x43170
    // (thunk 0x43220).
    void Kill() override;                // slot 74

    // Binds the generator to the patch and the request's render target. A
    // slave request registers the generator with its master; any other
    // request takes a bus generator when `createBus` is set. False when no
    // bus generator is available. At 0x42600.
    bool Setup(const ResourcePtr<FusionPatchResource>& patch, const PlayArgs& args, bool createBus);
    // Loads the patch at the path and plays it unless it failed to load. No
    // caller in this build. At 0x42930.
    void LoadAndSetPatch(const char* path);

    // 0x19C8508.
    static Symbol kTypeId;
    // Guards every generator's client list. 0x19C84F8.
    static CritSec mClientListLock;

    // Field names are not in the reference map.
    // The master instrument's handle while the generator is its slave.
    unsigned int mMasterHandle;
    // Renders the generator's bus unless it is a slave.
    AudioBusGenerator* mBusGenerator;
    ClientList mAudioThreadClients;
};

static_assert(offsetof(FusionGenerator, mMasterHandle) == 0x5470);
static_assert(offsetof(FusionGenerator, mBusGenerator) == 0x5478);
static_assert(offsetof(FusionGenerator, mAudioThreadClients) == 0x5480);
static_assert(sizeof(FusionGenerator) == 0x5498);

// The pool of Fusion generators. Every loaded FusionPatchResource registers
// itself under its file name, which play requests name. The vtable is at
// 0x18E0440; the object is 72 bytes.
class FusionGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into SoundManager::InitStandardGenManagers.
    explicit FusionGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The ids, created on first use and inlined into GetId and
    // GetResourceExt. The local statics are at 0x19C8510 and 0x19C8520.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("FusionGeneratorManager");
        }
        return id;
    }
    static Symbol ResExt() {
        static Symbol resExt;
        if (resExt == Symbol()) {
            resExt = Symbol(".fusion");
        }
        return resExt;
    }

    // Plays the registered patch the request names. At 0x423A0.
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0
    int GetIndex() override;               // slot 6: 0x43480
    Symbol GetId() override;               // slot 7: 0x43490
    Symbol GetResourceExt() override;      // slot 8: 0x43530, ".fusion"
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x435D0
    void SendStopToAllGenerators() override;  // slot 10: 0x43660
    void SendKillToAllGenerators() override;  // slot 11: 0x436D0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x43750
    void _SetManagerIndex(int index) override;      // slot 13: 0x438A0
    void _InitGeneratorPool() override;    // slot 14: 0x438B0
    bool _DeleteGeneratorPool() override;  // slot 15: 0x43A10
    // Slots 16-17 at 0x43B70, which jumps to the base destructor, and
    // 0x43B80.
    ~FusionGeneratorManager() override;

    // Takes an idle generator for another instrument, such as a
    // MultiFusion generator's part, bound to the target's voice pool and
    // reset. The map has GetFreeGenerator(AudioEmitter*); this build
    // passes the render target too. At 0x422C0.
    FusionGenerator* GetFreeGenerator(AudioRenderTarget* target, AudioEmitter* emitter);

    // Registers the patch under the name. At 0x41F90.
    static void AddPatch(Symbol name, FusionPatchResource* patch);
    // Appends the name of every registered patch. No caller in this build.
    // At 0x42090. Name not in the reference map.
    static void GetPatchNames(eastl::vector<Symbol>& names);
    // Removes every registration of the patch; false when it had none. At
    // 0x421F0.
    static bool RemovePatch(FusionPatchResource* patch);

    // "FusionGeneratorManager", at 0x19B00A0.
    static const char* kIdStr;
    // The registered patches by name, at 0x19C84C0.
    static eastl::map<Symbol, FusionPatchResource*> sPatchMap;
    // Guards sPatchMap. 0x19C84B0. Name not in the reference map.
    static CritSec sPatchMapCritSec;

    FusionGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(FusionGeneratorManager, mPool) == 64);
static_assert(sizeof(FusionGeneratorManager) == 72);
