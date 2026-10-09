#pragma once

#include <cstddef>

#include "math/transform/Transform.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class AudioEmitter;
class AudioGenerator;
class AudioGeneratorManager;
class Component;
class CompositeGenerator;
class CompositeGeneratorManager;
class DataArray;
class FusionGeneratorManager;
class TextStream;
class TransEntityResource;
class Vector3;
struct PlayArgs;

// Pool sizes and voice settings of the standard generator managers, copied
// into the sound manager by InitStandardGenManagers. The defaults are at
// 0x19C5810. Names not in the reference map.
struct GenManagerConfig {
    int mFmodAudioStreamPool;
    int mMoggPool;
    int mMoggMusicPool;
    int mFusionPool;
    // Passed to the default render target's InitVoicePool.
    int mHardVoiceLimit;
    int mSoftVoiceLimit;
    int mNumPitchShifters;
    // Also the voice pool's skipVoiceDecoders flag. Moggs then decode in
    // temporary memory (0x65290). The name is inferred.
    bool mPreDecodeMoggs;
    // Handed to the mogg decoder thread (0x10230); true by default. The
    // name is inferred and the evidence is weak.
    bool mThreadedMoggDecode = true;
    int mMultiFusionPool;
    int mSynthRackPool;
    int mMidiMusicPool;
    // The FMOD managers are built by the FMOD platform (slot 20 at
    // 0x261210). The Studio sound pool is inferred from its order.
    int mFmodStudioSoundPool;
    int mFmodAudioBusPool;
    int mMusicTimelinePool;
    int mFmodBufferedStreamPool;
    int mFmodDialogPool;
};

static_assert(offsetof(GenManagerConfig, mHardVoiceLimit) == 0x10);
static_assert(offsetof(GenManagerConfig, mPreDecodeMoggs) == 0x1C);
static_assert(offsetof(GenManagerConfig, mThreadedMoggDecode) == 0x1D);
static_assert(offsetof(GenManagerConfig, mMultiFusionPool) == 0x20);
static_assert(offsetof(GenManagerConfig, mFmodAudioBusPool) == 0x30);
static_assert(offsetof(GenManagerConfig, mFmodDialogPool) == 0x3C);
static_assert(sizeof(GenManagerConfig) == 64);

// The engine's sound system (audio/SoundManager.o): generator managers and
// their handles, the default 2D emitter and the joypad emitters, script
// sound functions and the FMOD platform's per-frame update. The single
// instance is theSoundManager at 0x19C5638; the vtable is at 0x18DC250 and
// the object is 288 bytes. This build has neither the map's VCA and master
// faders nor its CPU and bus-routing helpers.
class SoundManager : public MsgSink {
public:
    // The map's eastl::vector<SoundManager::JoypadEmitterAndListenerEntry>.
    // Field names are not in the reference map.
    struct JoypadEmitterAndListenerEntry {
        TransEntityResource* mResource;
        AudioEmitter* mEmitter;
        // The entity's listener component (class "AudioListener").
        Component* mListener;
    };

    SoundManager();            // 0x9D0
    ~SoundManager() override;  // slots 0-1: 0xB60, 0xBD0
    // Slot 2 at 0x8E00: forwards to the default emitter's component.
    DataNode Handle(DataArray* msg, bool warn) override;

    // Reads "fmod_output_type", "thread_poll_default_emitter" and
    // "master_pause_bus_path" from the configuration and starts the sound
    // system. At 0xC40; no caller was found. Name not in the reference map.
    static void Init(DataArray* config);
    // Starts FMOD, the resource and component types, the default emitter
    // and the composite generators, then registers the theSoundManager
    // script variable and the exit callback. At 0xD50. The map has the
    // member Init(bool); this build's static version takes the output type,
    // the threaded-poll flag and two FMOD platform arguments, and ignores
    // the configuration.
    static void Init(
        DataArray* config,
        int outputType,
        bool threadPollDefaultEmitter,
        void* platformReserved,
        void* externalStudioSystem);
    // The exit callback: shuts the mics, the sound manager and FMOD down.
    // At 0x14D0. The map has a member Terminate().
    static void Terminate();
    static void _InitAudioDataFuncs();  // 0xFE0. The map has a member.
    // Registers the audio component types and the FMOD platform's. At
    // 0x1130. Name not in the reference map.
    static void _InitComponents();
    // Creates the default 2D emitter's entity and, when asked, the thread
    // that polls it. At 0x1190. The map has _InitDefaultEmitter().
    void _InitDefaultEmitter(bool threadPoll);
    void _InitCompositeGenMgr();  // 0x1360, also inlined into Init.
    // Registers the emitter names of "sound_manager game_wide_emitter_names".
    // At 0x1430, also inlined into Init. Name not in the reference map.
    static void _InitGameWideEmitters();

    // The polling thread's entry: polls the default emitter's entity each
    // time Poll releases the semaphore. At 0x5B80. The map has
    // SoundPollFunc(); this one is the matching thread entry, so the
    // identification is weak.
    static int SoundPollFunc(void* context);

    AudioEmitter* GetDefault2DEmitter() const;  // 0x5C20
    // The default emitter's component. At 0x5C50. Name not in the reference
    // map.
    Component* GetDefault2DEmitterComponent() const;
    // Resizes the joypad emitter list, creating an emitter and listener
    // entity for each new entry. At 0x5C60.
    void InitJoypadEmitters(int count);
    // At 0x5EB0. The second argument is the listener's platform id.
    void SetJoypadEmitterPlatformId(int index, int platformId);
    // The entry's emitter, or the default emitter past the list. At 0x5ED0.
    AudioEmitter* GetJoypadEmitter(int index);
    // Whether the entry's listener is active. At 0x5F10.
    bool HasJoypadEmitter(int index);
    // Clears the default emitter when it is the given one. At 0x5F50. Name
    // not in the reference map.
    void ClearDefault2DEmitter(AudioEmitter* emitter);
    void SetDefault2DEmitter(AudioEmitter* emitter);  // 0x5F60. Name not in the reference map.
    // The language of localized banks, "eng" by default. Setting a new one
    // reloads the localized banks. Names not in the reference map.
    void SetLanguage(Symbol language);  // 0x5F70
    Symbol GetLanguage() const;         // 0x5FA0

    // The generator with the handle, retained, or null when the handle is
    // stale or names no manager.
    AudioGenerator* LockIfOwned(unsigned int handle);  // 0x5FB0

    // Copies the defaults and initializes with them. At 0x6000.
    void InitStandardGenManagers();
    // Creates the standard generator managers once. At 0x6010. Name not in
    // the reference map.
    void InitStandardGenManagers(const GenManagerConfig& config);
    // The default configuration. At 0x61A0. Name not in the reference map.
    static void GetDefaultGenManagerConfig(GenManagerConfig& config);
    // Creates a manager of the type and records it under the type's name.
    // Instantiated at 0x61C0 through 0x7220 and in the FMOD platform (for
    // example 0x261260).
    template <class T>
    static T* InitGeneratorManager(int poolSize);
    // Creates the manager with the class name; false for an unknown name.
    // At 0x7120.
    static bool InitGeneratorManager(const char* name, int poolSize);

    // Stores the listener transform and moves FMOD's listener. At 0x74B0.
    void UpdateActiveListener(const Transform& xfm, const Vector3& velocity);
    // Updates the emitters, the generator managers, the default render
    // target, the FMOD platform and the mics once per frame.
    void Poll();  // 0x7560
    // Polls theSoundManager and returns zero. At 0x7540; no caller was
    // found. Name not in the reference map.
    static int PollCallback();
    // Kills the default and joypad emitters' sounds. At 0x77C0, inlined into
    // _DoTerminate.
    void KillAllNonObjectSounds();
    // Kills the composite generators and polls until their pool is idle. At
    // 0x7630, inlined into _DoTerminate.
    void _DestroyCompositeGenerators();
    void _DoTerminate();  // 0x7680

    // Adds a generator manager and returns its index. The map has
    // _RegisterGeneratorManager(AudioGeneratorManager*, Symbol); this build
    // passes the manager's resource extension.
    int _RegisterGeneratorManager(AudioGeneratorManager* manager, Symbol ext);  // 0x7820
    // The FMOD platform's loaded events and buses. At 0x79C0 and 0x79E0.
    // The map has GetLoadedEvents(EntityPtr const&, eastl::vector<Symbol,
    // HmxAllocator::allocator>&); the arguments pass through untyped.
    void GetLoadedEvents(void* unused, void* events);
    void GetLoadedBuses(void* unused, void* buses);  // Name not in the reference map.
    // Plays the request on every generator manager after the composite one,
    // grouping several results under a composite generator. Returns the
    // handle, or zero.
    unsigned int PlaySound(const PlayArgs& args);  // 0x7A00
    unsigned int PlaySound(Symbol name, AudioEmitter* emitter, bool paused);  // 0x7C70
    // Pauses or resumes the bus (by default "master_pause_bus_path") and
    // records the state. At 0x7DC0. Name not in the reference map.
    void SetPaused(bool paused, const char* busPath, bool immediate);
    // Moves the generator's children to a new composite generator under the
    // default emitter and stops them, or kills the generator when none is
    // free. At 0x7E10.
    void ShutdownGenerator(CompositeGenerator& generator);
    // Stops the generator with the handle; false when the handle is stale.
    // At 0x7FD0. The map's version returns nothing.
    bool StopSound(unsigned int handle);
    // Stops the default and joypad emitters' sounds. At 0x8040.
    void StopAllNonObjectSounds();
    // The registered generator manager whose GetId matches, or null.
    AudioGeneratorManager* _GetManager(Symbol id);  // 0x8090
    // The "_get_loaded_events" handler body at 0x80E0: an array of the
    // platform's events for the object in the variable the message's third
    // node names, each as (symbol path path) with the leading '/' dropped
    // from the strings.
    DataNode _GetLoadedEvents(DataArray* msg);

    // Script functions registered by _InitAudioDataFuncs.
    static DataNode _OnPlaySound(DataArray* msg);           // 0x85E0, "play_sound"
    static DataNode _OnSoundValid(DataArray* msg);          // 0x8620, "sound_valid"
    static DataNode _OnSoundStop(DataArray* msg);           // 0x86A0, "sound_stop"
    static DataNode _OnSoundPause(DataArray* msg);          // 0x8730, "sound_pause"
    static DataNode _OnSoundContinue(DataArray* msg);       // 0x87C0, "sound_continue"
    static DataNode _OnSoundGetElapsedMs(DataArray* msg);   // 0x8850, "sound_get_elapsed_ms"
    static DataNode _OnSoundSetParameter(DataArray* msg);   // 0x88F0, "sound_set_param"

    // Calls each manager's GetId and takes and releases its lock, waiting
    // out a pool operation in progress. At 0x89D0; no caller was found.
    // Name not in the reference map; the evidence is weak.
    void SyncGeneratorManagers();
    // Prints the FMOD state, each generator manager's pool usage and every
    // named active sound. The map's signature is DumpGeneratorStats().
    void DumpGeneratorStats(TextStream& stream);  // 0x8A30

    // Two directories and the path FileMakePath builds from them; the
    // resource loader at 0x61C00 resolves its files against them. At 0x8D70
    // through 0x8DF0. Names not in the reference map; the evidence for
    // their meaning is weak.
    static void SetAudioPaths(const char* root, const char* dir);
    static const char* GetAudioFullPath();  // 0x8DD0
    static const char* GetAudioDirPath();   // 0x8DE0
    static const char* GetAudioRootPath();  // 0x8DF0

    // Managers created by InitGeneratorManager, by class name. At
    // 0x19C5850. Name not in the reference map.
    static eastl::map<Symbol, AudioGeneratorManager*> sGenManagers;
    // Created once by InitStandardGenManagers. At 0x19C5888. Name not in
    // the reference map.
    static FusionGeneratorManager* sFusionGenManager;
    // Registers a handle under PlayArgs::mGlobalSoundHandle; set to
    // StateGraphDriverCom::AddSoundHandle at 0x701620. At 0x19C5800. Name
    // not in the reference map.
    static void (*sAddSoundHandleFunc)(unsigned int handle, Symbol name);

    // Field names are not in the reference map.
    bool mStandardGenManagersInitialized;
    // Registered generator managers by resource extension.
    eastl::map<Symbol, AudioGeneratorManager*> mManagersByExt;
    // Registered generator managers; a handle's bits 24-30 index them.
    eastl::vector<AudioGeneratorManager*> mManagers;
    CompositeGeneratorManager* mCompositeGenMgr;
    TransEntityResource* mDefaultEmitterResource;
    AudioEmitter* mDefault2DEmitter;
    Transform mListenerXfm;
    Symbol mLanguage;
    eastl::vector<JoypadEmitterAndListenerEntry> mJoypadEmitters;
    bool mPaused;
    GenManagerConfig mGenManagerConfig;
};

static_assert(offsetof(SoundManager, mStandardGenManagersInitialized) == 8);
static_assert(offsetof(SoundManager, mManagersByExt) == 16);
static_assert(offsetof(SoundManager, mManagers) == 72);
static_assert(offsetof(SoundManager, mCompositeGenMgr) == 104);
static_assert(offsetof(SoundManager, mDefaultEmitterResource) == 112);
static_assert(offsetof(SoundManager, mDefault2DEmitter) == 120);
static_assert(offsetof(SoundManager, mListenerXfm) == 128);
static_assert(offsetof(SoundManager, mLanguage) == 176);
static_assert(offsetof(SoundManager, mJoypadEmitters) == 184);
static_assert(offsetof(SoundManager, mPaused) == 216);
static_assert(offsetof(SoundManager, mGenManagerConfig) == 220);
static_assert(sizeof(SoundManager) == 288);

extern SoundManager theSoundManager;

// The default emitter of theSoundManager. At 0x5C10, inlined into
// GetJoypadEmitter. Name not in the reference map.
AudioEmitter* GetDefaultAudioEmitter();
// Its component. At 0x5C30. Name not in the reference map.
Component* GetDefaultAudioEmitterComponent();

// The "master_pause_bus_path" bus SetPaused uses by default,
// "bus:/MASTER_PAUSE_BUS" unless the configuration names another. At
// 0x19B0090. Name not in the reference map.
extern const char* gMasterPauseBusPath;
