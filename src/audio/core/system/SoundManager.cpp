#include "audio/core/system/SoundManager.h"

#include <cstring>
#include <unistd.h>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/generators/CompositeGenerator.h"
#include "audio/core/generators/MoggGenerator.h"
#include "audio/core/fusion/FusionGenerator.h"
#include "audio/core/instruments/MultiFusionGenerator.h"
#include "audio/core/instruments/SynthRackGenerator.h"
#include "audio/core/music/MidiMusicGenerator.h"
#include "audio/core/resources/MidiMusicResource.h"
#include "audio/core/music/MoggMusicGenerator.h"
#include "audio/core/resources/MoggMusicResource.h"
#include "audio/core/music/MusicTimelineGenerator.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/resources/FusionPatchResource.h"
#include "audio/core/resources/MoggResource.h"
#include "audio/fmod/platform/orbis/FmodPlatform_PS4.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "entity/core/Entity.h"
#include "entity/core/TransEntityResource.h"
#include "mic/core/MicHwManager.h"
#include "os/debug/Debug.h"
#include "os/system/System.h"
#include "os/threading/Semaphore.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFunc.h"
#include "utl/options/Option.h"
#include "utl/text/MakeString.h"
#include "utl/text/TextStream.h"
#include "utl/threading/Thread.h"

// Engine functions the sound manager calls whose owners have not been
// reconstructed. Names not in the reference map unless noted.

// The map's OptionBool(char const*, bool); this build takes the list.
bool OptionBool(OptionArgs& args, const char* name, bool def);  // 0x2525A0
// The map's FileMakePath(char const*, char const*, char*). Weak: the body
// at 0x244960 was matched by its use, not read.
const char* FileMakePath(const char* root, const char* file, char* buffer);  // 0x244960

// Wrappers around the FMOD platform object in the render target registry's
// object.
void CreateFmodPlatform(bool liveUpdate, int outputType, void* reserved, void* studioSystem);  // 0xC0E80
void InitFmodPlatformResources(FmodPlatformInterface* platform);    // 0xC0E90, slot 18
void InitFmodPlatformComponents(FmodPlatformInterface* platform);   // 0xC0EA0, slot 19
void InitFmodPlatformGenManagers(FmodPlatformInterface* platform, const GenManagerConfig& config);  // 0xC0EB0, slot 20
// Removes the bus interface's premix callback from the default render
// target. Possibly the map's SoundManager::FadersControl::
// RemoveFmodPremixCallback(); the object is not identified.
void RemoveFmodBusPremixCallback(FmodBusInterface* bus);  // 0xBF790

// Mogg decoding options (stored at 0x19C8A28 and 0x19B00F8, read by the
// mogg loader at 0x65290) and the decoder thread's shutdown. Names inferred;
// the evidence is weak.
void SetMoggDecodeOptions(bool preDecode, bool threaded);  // 0x65620
void TerminateMoggDecodeThread();                          // 0x12ED0
// Records an emitter name the game keeps alive across entities. The
// emitter component's object.
void AddGameWideEmitterName(Symbol name);  // 0x35450

// The resource and component types whose registrations the sound manager
// runs. Each Init is inline in its class and emitted in this object: the
// resources at 0x1510 through 0x1F20 and the components at 0x2090 through
// 0x5730. Only these members are declared; the classes live in their own
// objects. The map names the classes' Init(); this build registers more
// types than the map's.
class WaveResource {
public:
    static void Init();  // 0x1510
};
class MoggSampleResource {
public:
    static void Init();  // 0x17F0
};
class MultiFusionResource {
public:
    static void Init();  // 0x1AD0
};
class MidiFileResource {
public:
    static void Init();  // 0x1C40
};
class AudioAnalyzerCom {
public:
    static void Init();  // 0x2090, class "AudioAnalyzer"
};
class AudioClipCom {
public:
    static void Init();  // 0x25D0, class "AudioClip"
};
class AudioClipMusicCom {
public:
    static void Init();  // 0x29B0, class "AudioClipMusic"
};
class AudioClipMidiMusicCom {
public:
    static void Init();  // 0x2DD0, class "AudioClipMidiMusic"
};
class AudioClipMoggMusicCom {
public:
    static void Init();  // 0x3200, class "AudioClipMoggMusic"
};
class AudioClipRepeatableCom {
public:
    static void Init();  // 0x3630, class "AudioClipRepeatable"
};
class AudioClipFusionCom {
public:
    static void Init();  // 0x3A40, class "AudioClipFusion"
};
class AudioBusEmitterCom {
public:
    static void Init();  // 0x3F20, class "AudioBusEmitter"
};
class DefaultEmitterProxyCom {
public:
    static void Init();  // 0x46D0, class "DefaultEmitterProxy"
};
class AudioListenerCom {
public:
    static void Init();  // 0x4B70, class "AudioListener"
    // Stores the platform (pad) id at +0x18. At 0x39AC0.
    void SetPlatformId(int platformId);
};
class PreloaderCom {
public:
    static void Init();  // 0x4F70, class "Preloader"
};
class MidiMsgBroadcasterCom {
public:
    static void Init();  // 0x5730, class "MidiMsgBroadcaster"
};

namespace {

// The emitter component as the sound manager reaches it. Field names are
// not in the reference map.
struct EmitterComponentView {
    unsigned char mComponentBase[56];
    // Set by the constructor (0x31D80) and cleared for the default emitter;
    // the component checks it when it releases its banks (0x33C70). The
    // meaning is not established.
    bool mReleaseBanks;
    unsigned char mRuntimeData[367];
    // The joypad emitter's listener component.
    Component* mListener;
    unsigned char mRuntimeDataTail[120];
    AudioEmitterCom mEmitter;
};

static_assert(offsetof(EmitterComponentView, mReleaseBanks) == 56);
static_assert(offsetof(EmitterComponentView, mListener) == 0x1A8);
static_assert(offsetof(EmitterComponentView, mEmitter) == 0x228);

// The listener component's fields that the joypad queries read. Names not
// in the reference map.
struct ListenerComponentView {
    unsigned char mComponentBase[24];
    int mPlatformId;
    bool mActive;
};

static_assert(offsetof(ListenerComponentView, mActive) == 0x1C);

EmitterComponentView* EmitterComponent(Component* component) {
    return reinterpret_cast<EmitterComponentView*>(component);
}

}  // namespace

// The class symbol of the listener component, at 0x19C7C88. Name not in the
// reference map.
extern Symbol gAudioListenerComClass;

// The globals at 0x19C5638 through 0x19C5E90, in the binary's order.
SoundManager theSoundManager;
// Names not in the reference map.
static bool sThreadPollDefaultEmitter;    // 0x19C5758
static Semaphore sDefaultEmitterPollSem;  // 0x19C575C
static bool sStopDefaultEmitterPoll;      // 0x19C5770
static NamedThread sDefaultEmitterPollThread;  // 0x19C5778
void (*SoundManager::sAddSoundHandleFunc)(unsigned int handle, Symbol name);
static GenManagerConfig sDefaultGenManagerConfig = {
    8, 8, 8, 16, 32, 28, 0, false, true, 16, 16, 16, 128, 32, 8, 8, 32,
};
eastl::map<Symbol, AudioGeneratorManager*> SoundManager::sGenManagers;
FusionGeneratorManager* SoundManager::sFusionGenManager;
static char sAudioRootPath[512];  // 0x19C5890
static char sAudioDirPath[512];   // 0x19C5A90
static char sAudioFullPath[512];  // 0x19C5C90
const char* gMasterPauseBusPath = "bus:/MASTER_PAUSE_BUS";

// Holds a generator retained by LockIfOwned and releases it on destruction.
// The map emits ~SoundHandleLock() in SoundManager.o; this build inlines
// it. The constructor's arguments are not in the reference map.
class SoundHandleLock {
public:
    SoundHandleLock(SoundManager& manager, unsigned int handle)
        : mGenerator(manager.LockIfOwned(handle)) {}
    ~SoundHandleLock() {
        if (mGenerator != nullptr) {
            --mGenerator->mRefCount;
        }
    }

    SoundHandleLock(const SoundHandleLock&) = delete;
    SoundHandleLock& operator=(const SoundHandleLock&) = delete;

    AudioGenerator* mGenerator;  // Name not in the reference map.
};

namespace {

// Handle fields as AudioGenerator::GetNewHandle packs them. Names not in the
// reference map.
constexpr unsigned int kHandleManagerShift = 24;
constexpr unsigned int kHandleManagerMask = 0x7F;
constexpr unsigned int kHandleIndexShift = 14;
constexpr unsigned int kHandleIndexMask = 0x3FF;

// The composite generators' pool size. Name not in the reference map.
constexpr int kCompositePoolSize = 256;
// Wait between polls while the composite pool drains. Name not in the
// reference map.
constexpr useconds_t kTerminatePollUs = 10000;

}  // namespace

// Reconstructed from eboot.elf at 0x9D0. The static initializer at 0xF410
// inlines the same sequence for theSoundManager.
SoundManager::SoundManager()
    : mStandardGenManagersInitialized(false),
      mCompositeGenMgr(nullptr),
      mDefault2DEmitter(nullptr),
      mListenerXfm(Transform::sID),
      mLanguage("eng"),
      mPaused(false),
      mGenManagerConfig() {}

// Reconstructed from eboot.elf at 0xB60 (complete) and 0xBD0 (deleting).
SoundManager::~SoundManager() {}

// Reconstructed from eboot.elf at 0xC40. The "fmod_live_update" entry is
// read and ignored; Init takes it from the command line.
void SoundManager::Init(DataArray* config) {
    bool liveUpdate = false;
    config->FindData(Symbol("fmod_live_update"), liveUpdate, false);
    bool threadPoll = false;
    config->FindData(Symbol("thread_poll_default_emitter"), threadPoll, false);
    int outputType = -1;
    config->FindData(Symbol("fmod_output_type"), outputType, false);
    const char* busPath = gMasterPauseBusPath;
    config->FindData(Symbol("master_pause_bus_path"), busPath, false);
    gMasterPauseBusPath = busPath;
    Init(nullptr, outputType, threadPoll, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0xD50.
void SoundManager::Init(
    DataArray* config,
    int outputType,
    bool threadPollDefaultEmitter,
    void* platformReserved,
    void* externalStudioSystem) {
    static_cast<void>(config);
    const bool liveUpdate = OptionBool(gOptionArgs, "fmod_live_update", false);
    if (outputType == -1) {
        outputType = 0;
    }
    _InitAudioDataFuncs();
    gMicHwManager.Init();
    CreateFmodPlatform(liveUpdate, outputType, platformReserved, externalStudioSystem);
    InitFmodPlatformResources(gFmodPlatformInterface);
    WaveResource::Init();
    MoggResource::Init();
    MoggSampleResource::Init();
    FusionPatchResource::Init();
    MultiFusionResource::Init();
    MidiFileResource::Init();
    MoggMusicResource::Init();
    MidiMusicResource::Init();
    _InitComponents();
    theSoundManager._InitDefaultEmitter(threadPollDefaultEmitter);
    theSoundManager._InitCompositeGenMgr();

    DataNode self;
    self.mValue.object = &theSoundManager;
    self.mType = kDataObject;
    DataSetGlobal(Symbol("theSoundManager"), self);

    _InitGameWideEmitters();
    TheDebug.AddExitCallback(Terminate);
}

// Reconstructed from eboot.elf at 0xFE0.
void SoundManager::_InitAudioDataFuncs() {
    DataRegisterFunc(Symbol("play_sound"), _OnPlaySound);
    DataRegisterFunc(Symbol("sound_valid"), _OnSoundValid);
    DataRegisterFunc(Symbol("sound_stop"), _OnSoundStop);
    DataRegisterFunc(Symbol("sound_pause"), _OnSoundPause);
    DataRegisterFunc(Symbol("sound_continue"), _OnSoundContinue);
    DataRegisterFunc(Symbol("sound_get_elapsed_ms"), _OnSoundGetElapsedMs);
    DataRegisterFunc(Symbol("sound_set_param"), _OnSoundSetParameter);
}

// Reconstructed from eboot.elf at 0x1130. The binary also registers
// AudioEmitterCom (0x42F0) after AudioBusEmitterCom and FusionPatchCom
// (0x5350) after PreloaderCom; their Init declarations are missing from
// the classes' headers, so those two calls are left out.
void SoundManager::_InitComponents() {
    AudioAnalyzerCom::Init();
    AudioClipCom::Init();
    AudioClipMusicCom::Init();
    AudioClipMidiMusicCom::Init();
    AudioClipMoggMusicCom::Init();
    AudioClipRepeatableCom::Init();
    AudioClipFusionCom::Init();
    AudioBusEmitterCom::Init();
    DefaultEmitterProxyCom::Init();
    AudioListenerCom::Init();
    PreloaderCom::Init();
    MidiMsgBroadcasterCom::Init();
    InitFmodPlatformComponents(gFmodPlatformInterface);
}

// Reconstructed from eboot.elf at 0x1190. The binary also touches the
// thread's entity state before loading, without using it.
void SoundManager::_InitDefaultEmitter(bool threadPoll) {
    mDefaultEmitterResource = new TransEntityResource();
    GameObject* object = mDefaultEmitterResource->CreateEntity()->CreateObject(0, 0);
    object->SetName(Symbol("default_audio_emitter"));
    EmitterComponentView* component =
        EmitterComponent(object->CreateComponent(gAudioEmitterComClass, false));
    mDefault2DEmitter = &component->mEmitter;
    mDefault2DEmitter->Set2D(true);
    component->mReleaseBanks = false;
    mDefaultEmitterResource->LoadResources();
    mDefaultEmitterResource->EnterEntity(mDefaultEmitterResource->mEntity);

    sThreadPollDefaultEmitter = threadPoll;
    if (threadPoll) {
        sDefaultEmitterPollSem.Create(0, 0);
        sStopDefaultEmitterPoll = false;
        const ThreadMap::TaskDesc* task = ThreadMap::GetTaskSettings("audio_2demitter_update");
        sDefaultEmitterPollThread.Create(
            SoundPollFunc,
            this,
            "def_emitter_poll",
            task->mProcessor,
            task->mPriority,
            task->mStackSize,
            task->mAffinityMask);
        sDefaultEmitterPollThread.mThread.Start();
    }
}

// Reconstructed from eboot.elf at 0x1360.
void SoundManager::_InitCompositeGenMgr() {
    auto* manager = new CompositeGeneratorManager(kCompositePoolSize);
    mCompositeGenMgr = manager;
    manager->Init();
}

// Reconstructed from eboot.elf at 0x1430.
void SoundManager::_InitGameWideEmitters() {
    DataArray* names = SystemConfig(Symbol("sound_manager"), Symbol("game_wide_emitter_names"));
    for (int i = 1; i < names->Size(); ++i) {
        AddGameWideEmitterName(names->Sym(i));
    }
}

// Reconstructed from eboot.elf at 0x14D0.
void SoundManager::Terminate() {
    gMicHwManager.Terminate();
    theSoundManager._DoTerminate();
    if (gFmodPlatformInterface != nullptr) {
        gFmodPlatformInterface->Terminate();
    }
}

// Reconstructed from eboot.elf at 0x5B80.
int SoundManager::SoundPollFunc(void* context) {
    auto* manager = static_cast<SoundManager*>(context);
    while (!sStopDefaultEmitterPoll) {
        sDefaultEmitterPollSem.Wait();
        if (sStopDefaultEmitterPoll) {
            break;
        }
        TransEntityResource* resource = manager->mDefaultEmitterResource;
        if (resource != nullptr) {
            resource->PollEntity(resource->mEntity);
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x5C10.
AudioEmitterCom* GetDefaultAudioEmitter() {
    return theSoundManager.mDefault2DEmitter;
}

// Reconstructed from eboot.elf at 0x5C20.
AudioEmitterCom* SoundManager::GetDefault2DEmitter() const {
    return mDefault2DEmitter;
}

// Reconstructed from eboot.elf at 0x5C30.
Component* GetDefaultAudioEmitterComponent() {
    return theSoundManager.mDefault2DEmitter->GetComponent();
}

// Reconstructed from eboot.elf at 0x5C50.
Component* SoundManager::GetDefault2DEmitterComponent() const {
    return mDefault2DEmitter->GetComponent();
}

// Reconstructed from eboot.elf at 0x5C60.
void SoundManager::InitJoypadEmitters(int count) {
    int size = static_cast<int>(mJoypadEmitters.size());
    if (size > count) {
        for (int i = size - 1; i >= count; --i) {
            if (mJoypadEmitters[i].mResource != nullptr) {
                delete mJoypadEmitters[i].mResource;
            }
            mJoypadEmitters[i].mResource = nullptr;
        }
        size = count;
    }
    mJoypadEmitters.resize(count);
    for (int i = size; i < count; ++i) {
        JoypadEmitterAndListenerEntry& entry = mJoypadEmitters[i];
        entry.mResource = new TransEntityResource();
        GameObject* object = entry.mResource->CreateEntity()->CreateObject(0, 0);
        EmitterComponentView* emitter =
            EmitterComponent(object->CreateComponent(gAudioEmitterComClass, false));
        entry.mEmitter = &emitter->mEmitter;
        entry.mEmitter->Set2D(true);
        entry.mListener = object->CreateComponent(gAudioListenerComClass, false);
        emitter->mListener = entry.mListener;
        entry.mResource->LoadResources();
        entry.mResource->EnterEntity(entry.mResource->mEntity);
    }
}

// Reconstructed from eboot.elf at 0x5EB0.
void SoundManager::SetJoypadEmitterPlatformId(int index, int platformId) {
    reinterpret_cast<AudioListenerCom*>(mJoypadEmitters[index].mListener)->SetPlatformId(platformId);
}

// Reconstructed from eboot.elf at 0x5ED0.
AudioEmitterCom* SoundManager::GetJoypadEmitter(int index) {
    if (index < static_cast<int>(mJoypadEmitters.size())) {
        return mJoypadEmitters[index].mEmitter;
    }
    return GetDefaultAudioEmitter();
}

// Reconstructed from eboot.elf at 0x5F10.
bool SoundManager::HasJoypadEmitter(int index) {
    if (index < 0 || index >= static_cast<int>(mJoypadEmitters.size())) {
        return false;
    }
    return reinterpret_cast<ListenerComponentView*>(mJoypadEmitters[index].mListener)->mActive;
}

// Reconstructed from eboot.elf at 0x5F50.
void SoundManager::ClearDefault2DEmitter(AudioEmitterCom* emitter) {
    if (mDefault2DEmitter == emitter) {
        mDefault2DEmitter = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x5F60.
void SoundManager::SetDefault2DEmitter(AudioEmitterCom* emitter) {
    mDefault2DEmitter = emitter;
}

// Reconstructed from eboot.elf at 0x5F70.
void SoundManager::SetLanguage(Symbol language) {
    if (mLanguage != language) {
        mLanguage = language;
        if (gFmodPlatformInterface != nullptr) {
            gFmodPlatformInterface->ReloadLocalizedBanks();
        }
    }
}

// Reconstructed from eboot.elf at 0x5FA0.
Symbol SoundManager::GetLanguage() const {
    return mLanguage;
}

// Reconstructed from eboot.elf at 0x5FB0. Zero and all-ones are never live
// handles.
AudioGenerator* SoundManager::LockIfOwned(unsigned int handle) {
    if (handle + 1 < 2) {
        return nullptr;
    }
    int managerIndex = static_cast<int>((handle >> kHandleManagerShift) & kHandleManagerMask);
    if (managerIndex >= static_cast<int>(mManagers.size())) {
        return nullptr;
    }
    return mManagers[managerIndex]->LockIfOwned(handle, (handle >> kHandleIndexShift) & kHandleIndexMask);
}

// Reconstructed from eboot.elf at 0x6000.
void SoundManager::InitStandardGenManagers() {
    InitStandardGenManagers(sDefaultGenManagerConfig);
}

// Reconstructed from eboot.elf at 0x6010.
void SoundManager::InitStandardGenManagers(const GenManagerConfig& config) {
    if (mStandardGenManagersInitialized) {
        return;
    }
    mGenManagerConfig = config;
    InitFmodPlatformGenManagers(gFmodPlatformInterface, config);
    InitGeneratorManager<MoggGeneratorManager>(config.mMoggPool);
    InitGeneratorManager<MoggMusicGeneratorManager>(config.mMoggMusicPool);
    if (sFusionGenManager == nullptr) {
        sFusionGenManager = new FusionGeneratorManager(config.mFusionPool);
        sFusionGenManager->Init();
        gAudioRenderTargets.mDefault->InitVoicePool(
            config.mHardVoiceLimit,
            config.mSoftVoiceLimit,
            config.mNumPitchShifters,
            config.mPreDecodeMoggs);
    }
    SetMoggDecodeOptions(config.mPreDecodeMoggs, config.mThreadedMoggDecode);
    InitGeneratorManager<MultiFusionGeneratorManager>(config.mMultiFusionPool);
    InitGeneratorManager<SynthRackGeneratorManager>(config.mSynthRackPool);
    InitGeneratorManager<MidiMusicGeneratorManager>(config.mMidiMusicPool);
    InitGeneratorManager<MusicTimelineGeneratorManager>(config.mMusicTimelinePool);
    mStandardGenManagersInitialized = true;
}

// Reconstructed from eboot.elf at 0x61A0.
void SoundManager::GetDefaultGenManagerConfig(GenManagerConfig& config) {
    config = sDefaultGenManagerConfig;
}

// Reconstructed from eboot.elf at 0x61C0 (Mogg), 0x6450 (MoggMusic),
// 0x66E0 (MultiFusion), 0x6970 (SynthRack), 0x6C00 (MidiMusic), 0x6E90
// (MusicTimeline) and 0x7220 (Fusion). A manager already recorded under the
// name is looked up and left in place; the check's report is compiled out.
template <class T>
T* SoundManager::InitGeneratorManager(int poolSize) {
    if (sGenManagers.find(Symbol(T::kIdStr)) != sGenManagers.end()) {
        static_cast<void>(sGenManagers[Symbol(T::kIdStr)]);
    }
    T* manager = new T(poolSize);
    manager->Init();
    sGenManagers[Symbol(T::kIdStr)] = manager;
    return manager;
}

// Reconstructed from eboot.elf at 0x7120.
bool SoundManager::InitGeneratorManager(const char* name, int poolSize) {
    if (std::strcmp(name, FusionGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<FusionGeneratorManager>(poolSize);
    } else if (std::strcmp(name, MidiMusicGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<MidiMusicGeneratorManager>(poolSize);
    } else if (std::strcmp(name, MoggGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<MoggGeneratorManager>(poolSize);
    } else if (std::strcmp(name, MoggMusicGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<MoggMusicGeneratorManager>(poolSize);
    } else if (std::strcmp(name, MultiFusionGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<MultiFusionGeneratorManager>(poolSize);
    } else if (std::strcmp(name, SynthRackGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<SynthRackGeneratorManager>(poolSize);
    } else if (std::strcmp(name, MusicTimelineGeneratorManager::kIdStr) == 0) {
        InitGeneratorManager<MusicTimelineGeneratorManager>(poolSize);
    } else {
        return false;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x74B0. The velocity reaches the platform
// unused.
void SoundManager::UpdateActiveListener(const Transform& xfm, const Vector3& velocity) {
    static_cast<void>(velocity);
    mListenerXfm = xfm;
    if (gFmodPlatformInterface != nullptr) {
        gFmodPlatformInterface->SetListenerXfm(xfm);
    }
}

// Reconstructed from eboot.elf at 0x7540.
int SoundManager::PollCallback() {
    theSoundManager.Poll();
    return 0;
}

// Reconstructed from eboot.elf at 0x7560.
void SoundManager::Poll() {
    if (sThreadPollDefaultEmitter) {
        sDefaultEmitterPollSem.Release();
    } else if (mDefaultEmitterResource != nullptr) {
        mDefaultEmitterResource->PollEntity(mDefaultEmitterResource->mEntity);
    }
    for (JoypadEmitterAndListenerEntry& entry : mJoypadEmitters) {
        entry.mResource->PollEntity(entry.mResource->mEntity);
    }
    for (AudioGeneratorManager* manager : mManagers) {
        manager->Poll();
    }
    if (gAudioRenderTargets.mDefault != nullptr) {
        gAudioRenderTargets.mDefault->Update();
    }
    if (gFmodPlatformInterface != nullptr) {
        gFmodPlatformInterface->Poll();
    }
    gMicHwManager.Poll();
}

// Reconstructed from eboot.elf at 0x7630.
void SoundManager::_DestroyCompositeGenerators() {
    mCompositeGenMgr->SendKillToAllGenerators();
    while (!mCompositeGenMgr->Destroy()) {
        usleep(kTerminatePollUs);
        Poll();
    }
}

// Reconstructed from eboot.elf at 0x7680.
void SoundManager::_DoTerminate() {
    KillAllNonObjectSounds();
    _DestroyCompositeGenerators();
    if (gFmodBusInterface != nullptr) {
        RemoveFmodBusPremixCallback(gFmodBusInterface);
    }
    if (sThreadPollDefaultEmitter) {
        sStopDefaultEmitterPoll = true;
        sDefaultEmitterPollSem.Release();
        sDefaultEmitterPollThread.mThread._Join();
    }
    if (mDefaultEmitterResource != nullptr) {
        delete mDefaultEmitterResource;
    }
    mDefaultEmitterResource = nullptr;
    mDefault2DEmitter = nullptr;
    for (JoypadEmitterAndListenerEntry& entry : mJoypadEmitters) {
        if (entry.mResource != nullptr) {
            delete entry.mResource;
        }
        entry.mResource = nullptr;
    }
    TerminateMoggDecodeThread();
}

// Reconstructed from eboot.elf at 0x77C0.
void SoundManager::KillAllNonObjectSounds() {
    if (mDefault2DEmitter == nullptr) {
        return;
    }
    mDefault2DEmitter->GetCompositeGenerator()->KillLocked();
    for (JoypadEmitterAndListenerEntry& entry : mJoypadEmitters) {
        entry.mEmitter->GetCompositeGenerator()->KillLocked();
    }
}

// Reconstructed from eboot.elf at 0x7820. The lookup of the manager in the
// list is left from a compiled-out check.
int SoundManager::_RegisterGeneratorManager(AudioGeneratorManager* manager, Symbol ext) {
    for (AudioGeneratorManager* registered : mManagers) {
        if (registered == manager) {
            break;
        }
    }
    mManagersByExt[ext] = manager;
    mManagers.push_back(manager);
    return static_cast<int>(mManagers.size()) - 1;
}

// Reconstructed from eboot.elf at 0x79C0.
void SoundManager::GetLoadedEvents(void* unused, void* events) {
    if (gFmodPlatformInterface != nullptr) {
        gFmodPlatformInterface->GetAllEventPaths(unused, events);
    }
}

// Reconstructed from eboot.elf at 0x79E0.
void SoundManager::GetLoadedBuses(void* unused, void* buses) {
    if (gFmodPlatformInterface != nullptr) {
        gFmodPlatformInterface->GetAllBusPaths(unused, buses);
    }
}

namespace {

// Groups two generators under a new composite generator for the emitter,
// starting it paused or playing. At 0x7B90. Name not in the reference map.
CompositeGenerator* GroupGenerators(
    CompositeGeneratorManager& manager,
    AudioEmitterCom* emitter,
    AudioGenerator* first,
    AudioGenerator* second,
    bool paused) {
    CompositeGenerator* composite = manager._AllocateGenerator(emitter);
    if (composite == nullptr) {
        return nullptr;
    }
    composite->AddGenerator(first);
    composite->AddGenerator(second);
    composite->mState = static_cast<AudioGenerator::State>(AudioGenerator::kStatePlaying + paused);
    return composite;
}

}  // namespace

// Reconstructed from eboot.elf at 0x7A00. Manager 0 is the composite
// manager, which plays nothing. When no composite generator is free, both
// generators are stopped under the default emitter.
unsigned int SoundManager::PlaySound(const PlayArgs& args) {
    CompositeGenerator* composite = nullptr;
    AudioGenerator* generator = nullptr;
    for (unsigned long i = 1; i < mManagers.size(); ++i) {
        AudioGenerator* played = mManagers[i]->Play(args);
        if (played == nullptr) {
            continue;
        }
        if (generator == nullptr) {
            generator = played;
        } else if (composite != nullptr) {
            composite->AddGenerator(played);
        } else {
            composite = GroupGenerators(*mCompositeGenMgr, args.mEmitter, generator, played, args.mStartPaused);
            if (composite == nullptr) {
                static_cast<CompositeGenerator*>(mDefault2DEmitter->GetCompositeGenerator())->AddGenerator(generator);
                generator->Stop();
                static_cast<CompositeGenerator*>(mDefault2DEmitter->GetCompositeGenerator())->AddGenerator(played);
                played->Stop();
                return 0;
            }
            generator = composite;
        }
    }
    if (generator == nullptr) {
        return 0;
    }
    AudioEmitterCom* emitter = args.mEmitter != nullptr ? args.mEmitter : mDefault2DEmitter;
    static_cast<CompositeGenerator*>(emitter->GetCompositeGenerator())->AddGenerator(generator);
    generator->mName = args.mName;
    const unsigned int handle = generator->mHandle;
    if (args.mGlobalSoundHandle != Symbol() && sAddSoundHandleFunc != nullptr) {
        sAddSoundHandleFunc(handle, args.mGlobalSoundHandle);
    }
    return handle;
}

// Reconstructed from eboot.elf at 0x7C70.
unsigned int SoundManager::PlaySound(Symbol name, AudioEmitterCom* emitter, bool paused) {
    PlayArgs args;
    args.mName = name;
    args.mEmitter = emitter;
    args.mStartPaused = paused;
    return PlaySound(args);
}

// Reconstructed from eboot.elf at 0x7DC0. A null path pauses the master
// pause bus.
void SoundManager::SetPaused(bool paused, const char* busPath, bool immediate) {
    if (gFmodBusInterface == nullptr) {
        return;
    }
    if (busPath == nullptr) {
        busPath = gMasterPauseBusPath;
    }
    if (gFmodBusInterface->SetBusPaused(paused, busPath, immediate)) {
        mPaused = paused;
    }
}

namespace {

// Moves the generator's children to a new composite generator without an
// emitter. At 0x7E90. Name not in the reference map.
CompositeGenerator* TakeOverChildren(CompositeGeneratorManager& manager, CompositeGenerator& generator) {
    CompositeGenerator* composite = manager._AllocateGenerator(nullptr);
    if (composite == nullptr) {
        return nullptr;
    }
    composite->_TakeOwnershipOfChildren(generator);
    return composite;
}

}  // namespace

// Reconstructed from eboot.elf at 0x7E10.
void SoundManager::ShutdownGenerator(CompositeGenerator& generator) {
    if (!generator.IsPlaying()) {
        return;
    }
    CompositeGenerator* orphans = TakeOverChildren(*mCompositeGenMgr, generator);
    if (orphans == nullptr) {
        generator.KillLocked();
        return;
    }
    static_cast<CompositeGenerator*>(mDefault2DEmitter->GetCompositeGenerator())->AddGenerator(orphans);
    orphans->Stop();
}

// Reconstructed from eboot.elf at 0x7FD0.
bool SoundManager::StopSound(unsigned int handle) {
    SoundHandleLock lock(*this, handle);
    if (lock.mGenerator == nullptr) {
        return false;
    }
    lock.mGenerator->Stop();
    return true;
}

// Reconstructed from eboot.elf at 0x8040.
void SoundManager::StopAllNonObjectSounds() {
    mDefault2DEmitter->GetCompositeGenerator()->Stop();
    for (JoypadEmitterAndListenerEntry& entry : mJoypadEmitters) {
        entry.mEmitter->GetCompositeGenerator()->Stop();
    }
}

// Reconstructed from eboot.elf at 0x8090.
AudioGeneratorManager* SoundManager::_GetManager(Symbol id) {
    for (AudioGeneratorManager* manager : mManagers) {
        if (manager->GetId() == id) {
            return manager;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x80E0.
DataNode SoundManager::_GetLoadedEvents(DataArray* msg) {
    void* object = msg->Node(2).Var(msg)->LiteralSink(nullptr);
    eastl::vector<Symbol> events;
    GetLoadedEvents(object, &events);
    DataArrayPtr result(new DataArray(0));
    for (Symbol* event = events.begin(); event != events.end(); ++event) {
        DataArrayPtr entry(new DataArray(3));
        entry->Node(0) = DataNode(*event);
        String path(event->Str());
        if (path.c_str()[0] == '/') {
            path.erase(0, 1);
        }
        entry->Node(1) = DataNode(path);
        entry->Node(2) = DataNode(path);
        result->Insert(result->Size(), DataNode(entry));
    }
    return DataNode(result);
}

// Reconstructed from eboot.elf at 0x85E0.
DataNode SoundManager::_OnPlaySound(DataArray* msg) {
    return DataNode(static_cast<int>(theSoundManager.PlaySound(msg->Sym(1), nullptr, false)));
}

// Reconstructed from eboot.elf at 0x8620.
DataNode SoundManager::_OnSoundValid(DataArray* msg) {
    SoundHandleLock lock(theSoundManager, static_cast<unsigned int>(msg->Int(1)));
    return DataNode(lock.mGenerator != nullptr ? 1 : 0);
}

// Reconstructed from eboot.elf at 0x86A0.
DataNode SoundManager::_OnSoundStop(DataArray* msg) {
    SoundHandleLock lock(theSoundManager, static_cast<unsigned int>(msg->Int(1)));
    if (lock.mGenerator != nullptr) {
        lock.mGenerator->Stop();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8730.
DataNode SoundManager::_OnSoundPause(DataArray* msg) {
    SoundHandleLock lock(theSoundManager, static_cast<unsigned int>(msg->Int(1)));
    if (lock.mGenerator != nullptr) {
        lock.mGenerator->Pause();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x87C0.
DataNode SoundManager::_OnSoundContinue(DataArray* msg) {
    SoundHandleLock lock(theSoundManager, static_cast<unsigned int>(msg->Int(1)));
    if (lock.mGenerator != nullptr) {
        lock.mGenerator->Continue();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x8850.
DataNode SoundManager::_OnSoundGetElapsedMs(DataArray* msg) {
    float elapsedMs = 0.0F;
    SoundHandleLock lock(theSoundManager, static_cast<unsigned int>(msg->Int(1)));
    if (lock.mGenerator != nullptr) {
        elapsedMs = lock.mGenerator->GetElapsedMs();
    }
    return DataNode(elapsedMs);
}

// Reconstructed from eboot.elf at 0x88F0.
DataNode SoundManager::_OnSoundSetParameter(DataArray* msg) {
    const unsigned int handle = static_cast<unsigned int>(msg->Int(1));
    const Symbol parameter = msg->Sym(2);
    const float value = msg->Float(3);
    SoundHandleLock lock(theSoundManager, handle);
    if (lock.mGenerator != nullptr) {
        lock.mGenerator->SetParameter(parameter, value);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x89D0.
void SoundManager::SyncGeneratorManagers() {
    for (AudioGeneratorManager* manager : mManagers) {
        manager->GetId();
        scePthreadMutexLock(&manager->mCritSec.mCritSec);
        scePthreadMutexUnlock(&manager->mCritSec.mCritSec);
    }
}

// Reconstructed from eboot.elf at 0x8A30. The pool counts are read under
// the manager's mutex without its entry count. Symbols print through
// TextStream's Symbol operator at 0x258900, which is not declared; the
// string operator prints the same text.
void SoundManager::DumpGeneratorStats(TextStream& stream) {
    if (gFmodPlatformInterface != nullptr) {
        gFmodPlatformInterface->PrintStats(stream);
    }
    stream << "\nActive generators:\n";
    for (AudioGeneratorManager* manager : mManagers) {
        scePthreadMutexLock(&manager->mCritSec.mCritSec);
        const int numFree = static_cast<int>(manager->mFreeList.mSize);
        scePthreadMutexUnlock(&manager->mCritSec.mCritSec);
        const int poolSize = manager->mPoolSize;
        stream << " " << manager->GetId() << ": " << (poolSize - numFree) << "/" << poolSize << "\n";
    }
    stream << "\nActive sounds:\n";
    eastl::vector<unsigned int> handles;
    for (AudioGeneratorManager* manager : mManagers) {
        manager->GetActiveHandles(&handles);
        for (unsigned int handle : handles) {
            SoundHandleLock lock(theSoundManager, handle);
            if (lock.mGenerator != nullptr && lock.mGenerator->mName != Symbol()) {
                FormatString text("%08x");
                text << handle;
                stream << " " << lock.mGenerator->mName << " (" << text.Str() << ")\n";
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x8D70.
void SoundManager::SetAudioPaths(const char* root, const char* dir) {
    std::strncpy(sAudioRootPath, root, sizeof(sAudioRootPath));
    sAudioRootPath[sizeof(sAudioRootPath) - 1] = '\0';
    std::strncpy(sAudioDirPath, dir, sizeof(sAudioDirPath));
    sAudioDirPath[sizeof(sAudioDirPath) - 1] = '\0';
    FileMakePath(root, dir, sAudioFullPath);
}

// Reconstructed from eboot.elf at 0x8DD0.
const char* SoundManager::GetAudioFullPath() {
    return sAudioFullPath;
}

// Reconstructed from eboot.elf at 0x8DE0.
const char* SoundManager::GetAudioDirPath() {
    return sAudioDirPath;
}

// Reconstructed from eboot.elf at 0x8DF0.
const char* SoundManager::GetAudioRootPath() {
    return sAudioRootPath;
}

// Reconstructed from eboot.elf at 0x8E00. The message name is read and
// unused; the handlers that tested it are compiled out.
DataNode SoundManager::Handle(DataArray* msg, bool warn) {
    static_cast<void>(msg->Sym(1));
    // The component's vtable begins with MsgSink's slots.
    return reinterpret_cast<MsgSink*>(GetDefault2DEmitterComponent())->Handle(msg, warn);
}
