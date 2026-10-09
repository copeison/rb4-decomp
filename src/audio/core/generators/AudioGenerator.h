#pragma once

#include <atomic>
#include <cstddef>
#include <functional>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/system/Audio.h"
#include "audio/core/system/SoundManager.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"
#include "os/threading/CritSec.h"

class AudioEmitterCom;
class AudioGenerator;
class AudioGeneratorManager;
class AudioRenderTarget;
class Component;
struct DialogPlayArgs;
struct PlayMusicArgs;
class TempoListener;
class TextStream;
class Transform;

// Music request options of the map's AudioEmitterCom::PlayMusic overloads.
// Their values are not modelled.
enum MusicSyncOptions : int;
enum MusicTimelineMapping : int;
enum MusicUnmutePoint : int;

// Playback request shared by every generator manager. The constructor and destructor are
// inlined into each builder, for example
// AudioGeneratorManager::Play(Symbol, AudioEmitterCom*, bool) at 0x40570.
// Field names are not in the reference map.
struct PlayArgs {
    struct ParameterValue {
        Symbol mName;
        float mValue;
    };

    using ParameterList = eastl::vector<ParameterValue>;

    enum Route : int {
        kRouteDefault = 0,
        kRouteEvent = 1,
        kRouteBus = 2,
    };

    PlayArgs()
        : mEmitter(nullptr),
          mStartPaused(false),
          mStartMuted(false),
          mHasInitialGain(false),
          mInitialGainDb(0.0F),
          mInitialGainFadeSecs(1.0F),
          mInitialGainPostFade(0),
          mRoute(kRouteDefault),
          mSpread(180.0F),
          mParameters(nullptr),
          mFormat(0),
          mOwnsParameters(false) {}
    ~PlayArgs() {
        if (mOwnsParameters) {
            delete mParameters;
            mParameters = nullptr;
            mOwnsParameters = false;
        }
    }

    Symbol mName;
    AudioEmitterCom* mEmitter;
    bool mStartPaused;
    bool mStartMuted;
    // The gain fields start a new 4-byte group and the route a new 8-byte
    // group; the bytes before each are never written by any builder, so the
    // original probably grouped them in nested structs.
    alignas(4) bool mHasInitialGain;
    float mInitialGainDb;
    float mInitialGainFadeSecs;
    int mInitialGainPostFade;
    alignas(8) int mRoute;
    Symbol mRoutePath;
    float mSpread;
    Symbol mRenderTarget;
    // When set, SoundManager's play (0x7A00) registers the new handle under
    // this name through the static StateGraphDriverCom::AddSoundHandle at
    // 0x701970, for the global lookups of HasSoundHandle.
    Symbol mGlobalSoundHandle;
    // Initialized to the empty symbol by every builder and read nowhere in
    // this build; the name only records that. The evidence is weak.
    Symbol mReservedName;
    ParameterList* mParameters;
    // 4 marks a DialogPlayArgs.
    int mFormat;
    bool mOwnsParameters;
    bool mStreaming;
};

static_assert(offsetof(PlayArgs, mStartPaused) == 16);
static_assert(offsetof(PlayArgs, mHasInitialGain) == 20);
static_assert(offsetof(PlayArgs, mInitialGainDb) == 24);
static_assert(offsetof(PlayArgs, mInitialGainPostFade) == 32);
static_assert(offsetof(PlayArgs, mRoute) == 40);
static_assert(offsetof(PlayArgs, mRoutePath) == 48);
static_assert(offsetof(PlayArgs, mSpread) == 56);
static_assert(offsetof(PlayArgs, mRenderTarget) == 64);
static_assert(offsetof(PlayArgs, mGlobalSoundHandle) == 72);
static_assert(offsetof(PlayArgs, mReservedName) == 80);
static_assert(offsetof(PlayArgs, mParameters) == 88);
static_assert(offsetof(PlayArgs, mFormat) == 96);
static_assert(offsetof(PlayArgs, mStreaming) == 101);
static_assert(sizeof(PlayArgs) == 104);

// The emitter interface of the audio emitter component. The component's
// RuntimeData (constructed at 0x37710, at +104 in the component) holds it at
// +0x228 in the component, with a back pointer to the component after it;
// the vtable is at 0x18DF628. Most slots forward to the component's own
// methods, whose names come from the map's AudioEmitterCom; the slots the
// map does not name are marked. Slot 18 onward drive the component's
// "is_2D" (+0x20) and "allow_dialog_overlap" (+0x21) properties and its
// dialog handle (+0x1B4) and interruptible flag (+0x1B8).
class AudioEmitterCom {
public:
    // Slot 0 at 0x379D0: the component's CompositeGenerator (at +0xE8), the
    // parent of every sound the emitter plays. Name not in the reference
    // map.
    virtual AudioGenerator* GetCompositeGenerator();
    // Slot 1 at 0x379E0: adds a tempo listener and sends it the current
    // tempo. AudioGenerator::RegisterTempoListener forwards here.
    virtual void RegisterTempoListener(TempoListener* listener);
    // Slot 2 at 0x379F0: false when the listener was not registered. Name
    // not in the reference map.
    virtual bool UnregisterTempoListener(TempoListener* listener);
    // Slots 3-10 at 0x37A00 and 0x37AB0 through 0x37B10 return the new
    // handle.
    virtual unsigned int PlaySound(PlayArgs& args);
    virtual unsigned int PlaySound(Symbol name);
    virtual unsigned int PrepareSound(Symbol name);
    virtual unsigned int PlayMusic(PlayMusicArgs& args);
    virtual unsigned int PlayMusic(
        Symbol name, MusicSyncOptions sync, MusicTimelineMapping mapping, MusicUnmutePoint unmute);
    virtual unsigned int PlayMusic(Symbol name, Symbol sync);
    virtual unsigned int PrepareMusic(Symbol name, MusicSyncOptions sync);
    virtual unsigned int PrepareMusic(Symbol name, Symbol sync);
    // Slots 11-14 at 0x37B20 through 0x37B80 stop, kill, pause and continue
    // the composite generator. RecordingAudioRenderTarget kills its emitter's
    // sounds through slot 0 instead.
    virtual void StopAllSounds();
    virtual void KillAllSounds();
    virtual void PauseAllSounds();
    virtual void ContinueAllSounds();
    // Slot 15 at 0x37BA0: the handle of the music the emitter follows.
    virtual unsigned int GetMasterMusic();
    // Slot 16 at 0x37BB0: the emitter's mix group, or null. When present,
    // the FMOD generators route through its channel group, whose getter is
    // inlined as null in this build. Name not in the reference map.
    virtual void* GetMixGroup();
    // Slot 17 at 0x37BC0: the component's world transform, or the listener's
    // for a 2D emitter. Name not in the reference map.
    virtual const Transform& GetWorldXfm();
    // Slot 18 at 0x37BE0: builds a DialogPlayArgs and plays it, stopping the
    // current line unless overlap is allowed. The last argument is an object
    // reference (the map's ObjPtr). Name not in the reference map.
    virtual unsigned int PlayDialog(
        Symbol name,
        bool interruptible,
        const std::function<void(AudioEmitterCom*, Symbol, void*)>& sink,
        const void* object);
    // Slot 19 at 0x37CB0: the same for a prepared request. Name not in the
    // reference map.
    virtual unsigned int PlayDialog(DialogPlayArgs& args);
    // Slot 20 at 0x37CC0: whether the dialog handle still names a generator
    // ("dialog_is_playing"). Name not in the reference map.
    virtual bool IsDialogPlaying();
    // Slot 21 at 0x37CD0: a dialog handle is held and its line is not interruptible.
    // Name not in the reference map.
    virtual bool IsDialogUninterruptible();
    // Slot 22 at 0x37CE0: sets the current line's interruptible flag when the handle
    // names a dialog generator. Name not in the reference map.
    virtual bool SetDialogInterruptible(bool interruptible);
    // Slot 23 at 0x37D00: the "allow_dialog_overlap" property. Name not in the
    // reference map.
    virtual void SetAllowDialogOverlap(bool allow);
    // Slots 24-27 at 0x37D20 through 0x37D50 read and write the "is_2D"
    // property; a 2D emitter is not
    // positioned. Names not in the reference map.
    virtual bool Is2D();
    virtual bool Is3D();
    virtual void Set2D(bool is2D);
    virtual void Set3D(bool is3D);
    // Slot 28 at 0x37D60: the owning component. Name not in the reference
    // map.
    virtual Component* GetComponent();
};

// The class symbol of the emitter component, at 0x19C7770. The component's
// registration at 0x32110 names the class "AudioEmitterCom"; the interface
// above sits at +0x228 in it. Name not in the reference map.
extern Symbol gAudioEmitterComClass;

// Linear gain ramp advanced by each generator's Poll. Its members are
// inlined at every use, for example in FmodAudioStreamGenerator::SetGain at
// 0x269D40. Names not in the reference map.
struct GainRamp {
    GainRamp() : mDurationMs(1000.0F), mProgress(1.0F), mBusy(false), mOnDone(nullptr) {}

    // A zero duration completes the ramp at once.
    void SetDurationMs(float ms) {
        if (ms == 0.0F) {
            if (!mBusy) {
                Complete();
            }
        } else {
            mDurationMs = ms;
        }
    }
    void Begin(float target) {
        mBusy = true;
        mStart = mCurrent;
        mTarget = target;
        mProgress = 0.0F;
        mElapsedMs = 0.0F;
        mBusy = false;
        mOnDone = nullptr;
        mOnDoneContext = nullptr;
    }
    // Jumps to the target without running the completion callback.
    void Snap() {
        if (!mBusy) {
            mCurrent = mTarget;
            mProgress = 1.0F;
            mOnDone = nullptr;
            mOnDoneContext = nullptr;
        }
    }
    void Complete() {
        mCurrent = mTarget;
        mProgress = 1.0F;
        if (mOnDone != nullptr) {
            mOnDone(mOnDoneContext);
        }
        mOnDone = nullptr;
        mOnDoneContext = nullptr;
    }
    bool Done() const {
        return mProgress == 1.0F;
    }
    void Advance(float elapsedMs) {
        mElapsedMs += elapsedMs;
        mProgress = mElapsedMs / mDurationMs;
        if (mProgress < 1.0F) {
            mCurrent = mStart + (mTarget - mStart) * mProgress;
        } else if (!mBusy) {
            Complete();
        }
    }

    float mStart;
    float mCurrent;
    float mTarget;
    float mDurationMs;
    float mProgress;
    float mElapsedMs;
    bool mBusy;
    void (*mOnDone)(void* context);
    void* mOnDoneContext;
};

static_assert(offsetof(GainRamp, mBusy) == 24);
static_assert(offsetof(GainRamp, mOnDone) == 32);
static_assert(sizeof(GainRamp) == 48);

// Minimum gain ramp, in milliseconds, at 0x124D444. Name not in the
// reference map.
constexpr float kMinGainRampMs = 25.0F;

// Pausable cycle timer, inlined as Hmx::Timer's start and stop sequences. A
// negative run count marks a paused timer. Names not in the reference map.
struct GeneratorTimer {
    void Reset() {
        if (mRunning > 0 && --mRunning == 0) {
            mElapsedCycles += __builtin_ia32_rdtsc() - mStartCycles;
        }
        mElapsedCycles = 0;
        mRunning = 0;
    }
    void Start() {
        if (mRunning >= 0 && mRunning++ == 0) {
            mStartCycles = __builtin_ia32_rdtsc();
        }
    }
    // Ends the outermost run, adding its cycles.
    void Stop() {
        if (mRunning > 0 && --mRunning == 0) {
            mElapsedCycles += __builtin_ia32_rdtsc() - mStartCycles;
        }
    }
    void Pause() {
        if (mRunning > 0) {
            mRunning = -mRunning;
            mElapsedCycles += __builtin_ia32_rdtsc() - mStartCycles;
        }
    }
    void Resume() {
        if (mRunning < 0) {
            mRunning = -mRunning;
            mStartCycles = __builtin_ia32_rdtsc();
        }
    }
    unsigned long Cycles() {
        if (mRunning > 0) {
            const unsigned long now = __builtin_ia32_rdtsc();
            mElapsedCycles += now - mStartCycles;
            mStartCycles = now;
        }
        return mElapsedCycles;
    }

    unsigned long mStartCycles;
    unsigned long mElapsedCycles;
    int mRunning;
};

static_assert(sizeof(GeneratorTimer) == 24);

// Base of every pooled voice. The vtable is at 0x18DCD58; the object is 80
// bytes.
class AudioGenerator {
public:
    // Inlined into every pool constructor, for example at 0x26A690 and
    // AudioBusGenerator's at 0xE0490.
    AudioGenerator()
        : mManager(nullptr),
          mIndex(0),
          mState(kStateStopped),
          mRefCount(0),
          mHandle(0xFFFFFFFF),
          mEmitter(nullptr) {}

    // Playback state at +28. Names not in the reference map.
    enum State : int {
        kStateInit = 0,
        kStateReady = 2,
        kStatePlaying = 3,
        kStatePaused = 4,
        kStateStopped = 5,
        kStateStopping = 6,
    };

    enum PostFadeOption : int {
        kPostFadeNone = 0,  // Name not in the reference map.
        kPostFadeStop = 1,  // Name not in the reference map.
    };

    virtual void Pause() = 0;                         // slot 0
    virtual void Continue() = 0;                      // slot 1
    virtual void Stop() = 0;                          // slot 2
    virtual State GetState();                         // slot 3: 0xE3E0. Name not in the reference map.
    virtual float GetElapsedMs() = 0;                 // slot 4
    virtual float GetTimelineMs() = 0;                // slot 5
    virtual float GetLengthMs() const;                // slot 6: 0xE3F0
    virtual void SeekToMs(float ms) = 0;              // slot 7
    virtual void SetSpeed(float speed, bool immediate);  // slot 8: 0xE400
    virtual float GetSpeed(bool* changing);           // slot 9: 0xE410
    virtual bool SetParameter(Symbol name, float value);    // slot 10: 0xE420
    virtual bool GetParameter(Symbol name, float& value);   // slot 11: 0xE430
    virtual void SetGain(float gain, float fadeSecs, PostFadeOption option);  // slot 12: 0xE440
    virtual float GetGain() const;                    // slot 13: 0xE450
    // Slot 14 at 0xE460. The map has SetMute(bool); this build passes an
    // immediate flag as well.
    virtual void SetMute(bool mute, bool immediate);
    virtual bool GetMute() const;                     // slot 15: 0xE470
    virtual bool IsMusic();                           // slot 16: 0xE480
    virtual bool IsInstrument();                      // slot 17: 0xE490
    virtual bool IsDialog();                          // slot 18: 0xE4A0. Name not in the reference map.
    virtual void Init(AudioGeneratorManager* manager, int index);  // slot 19: 0xE4B0
    virtual ~AudioGenerator();                        // slots 20-21: 0xE4E0, 0xE550
    virtual bool Poll() = 0;                          // slot 22
    virtual void Release() = 0;                       // slot 23
    virtual void* GetPluginData(const char* name);    // slot 24: 0xE5C0. Name not in the reference map.
    // Slots 25-26 at 0xE5D0 and 0xE5E0: a scale defaulting to one that a
    // Fusion play request sets and the music generators forward to their
    // streams; the base ignores it. Its meaning is not established. Names not
    // in the reference map.
    virtual void SetPlayScale(float scale);
    virtual float GetPlayScale();
    // Slot 27 at 0xE5F0: zero here; MoggGenerator returns a value of its
    // first stream. Name not in the reference map; the evidence is weak.
    virtual float GetPrimaryStreamValue();
    virtual void _InitTypeId() = 0;                   // slot 28
    virtual void Kill() = 0;                          // slot 29
    virtual AudioGenerator* GetGeneratorOfType(Symbol type) = 0;  // slot 30
    virtual Symbol GetTypeId() = 0;                   // slot 31. Name not in the reference map.

    // Builds an active handle from the generation counter, the manager index
    // and the pool index. At 0x40720.
    unsigned int GetNewHandle();
    // Calls Kill under the global generator lock. At 0x406D0. Name not in
    // the reference map.
    void KillLocked();
    // Clears the active handle bit when no caller holds the generator, so
    // the pool can release it. At 0x40770. Name not in the reference map.
    bool TryDeactivateHandle();
    // Registers a tempo listener, such as an HMX DSP plugin bound to this
    // generator, with the emitter. At 0x407C0.
    void RegisterTempoListener(TempoListener* listener);
    // Removes the listener from the emitter; false without one. At 0x407E0,
    // with no callers in this build. The map's version is larger; this one
    // only forwards to the emitter.
    bool UnregisterTempoListener(TempoListener* listener);

    // Field names are not in the reference map.
    // The sound's name, stored by SoundManager's play (0x7A00).
    Symbol mName;
    AudioGeneratorManager* mManager;
    int mIndex;
    State mState;
    std::atomic<int> mRefCount;
    unsigned int mHandle;
    LinkedListSizeTracked::Node mPoolNode;
    AudioEmitterCom* mEmitter;
    AudioRenderTarget* mRenderTarget;
};

static_assert(offsetof(AudioGenerator, mManager) == 16);
static_assert(offsetof(AudioGenerator, mState) == 28);
static_assert(offsetof(AudioGenerator, mRefCount) == 32);
static_assert(offsetof(AudioGenerator, mHandle) == 36);
static_assert(offsetof(AudioGenerator, mPoolNode) == 40);
static_assert(offsetof(AudioGenerator, mEmitter) == 64);
static_assert(offsetof(AudioGenerator, mRenderTarget) == 72);
static_assert(sizeof(AudioGenerator) == 80);

// Handle bits shared by every generator pool. Names not in the reference map.
constexpr unsigned int kGeneratorHandleActive = 0x80000000;

// Owner of one fixed generator pool (audio/AudioGenerator.o). The vtable is
// at 0x18DFF18.
class AudioGeneratorManager {
public:
    using GeneratorList =
        LinkedListSizeTracked::List<AudioGenerator, &AudioGenerator::mPoolNode>;

    virtual AudioGenerator* Play(const PlayArgs& args) = 0;  // slot 0
    virtual AudioGenerator* Play(Symbol name, AudioEmitterCom* emitter, bool paused);  // slot 1: 0x40570
    virtual AudioGenerator* Prepare(Symbol name, AudioEmitterCom* emitter);  // slot 2: 0x406C0
    virtual void Init();                              // slot 3: 0x40500
    virtual bool Destroy();                           // slot 4: 0x40540
    virtual void Poll() {}                            // slot 5: 0xDD20
    virtual int GetIndex() = 0;                       // slot 6
    virtual Symbol GetId() = 0;                       // slot 7
    virtual Symbol GetResourceExt() = 0;              // slot 8
    virtual AudioGenerator* LockIfOwned(unsigned int handle, int index) = 0;  // slot 9
    virtual void SendStopToAllGenerators() = 0;       // slot 10
    virtual void SendKillToAllGenerators() = 0;       // slot 11
    // Slot 12 fills an EASTL vector of handles. Name not in the reference map.
    virtual void GetActiveHandles(void* handles) = 0;
    virtual void _SetManagerIndex(int index) = 0;     // slot 13
    virtual void _InitGeneratorPool() = 0;            // slot 14
    virtual bool _DeleteGeneratorPool() = 0;          // slot 15
    // Slots 16-17: 0x40AC0, which jumps to the body at 0xE780, and 0x40AD0.
    virtual ~AudioGeneratorManager();

    // Returns an allocated generator of this pool to the free list. Inlined
    // where a music manager's Play cannot complete a voice, for example at
    // 0x449ED in MidiMusicGeneratorManager::Play; each copy also loads the
    // generator's reference count without using it.
    void FreeGenerator(AudioGenerator* generator) {
        ScopedCritSec lock(mCritSec);
        generator->mHandle &= ~kGeneratorHandleActive;
        generator->mEmitter = nullptr;
        if (generator->mPoolNode.mList != &mFreeList) {
            mFreeList.PushBack(*generator);
        }
    }

    // Field names are not in the reference map. Each concrete manager stores
    // its typed pool array at +64.
    int mPoolSize;
    CritSec mCritSec;
    GeneratorList mFreeList;
    int mManagerIndex;
};

static_assert(offsetof(AudioGeneratorManager, mPoolSize) == 8);
static_assert(offsetof(AudioGeneratorManager, mCritSec) == 16);
static_assert(offsetof(AudioGeneratorManager, mFreeList) == 32);
static_assert(offsetof(AudioGeneratorManager, mManagerIndex) == 56);
static_assert(sizeof(AudioGeneratorManager) == 64);

// Locked by AudioGenerator::KillLocked. Name not in the reference map.
extern CritSec gGeneratorKillCritSec;  // 0x19C8488

// One FMOD Studio event parameter as the platform reports it. Names not in
// the reference map.
struct EventParameterInfo {
    Symbol mName;
    float mMinimum;
    float mMaximum;
    float mDefault;
};

static_assert(sizeof(EventParameterInfo) == 24);

// Event parameter queries forwarded to the FMOD platform object; each
// returns zero without one. Names not in the reference map.
bool GetEventParameterDefault(const char* event, const char* parameter, float* value);  // 0x40800
int GetEventParameterCount(const char* event);  // 0x40830
bool GetEventParameterByIndex(const char* event, int index, EventParameterInfo* info);  // 0x40860
bool GetEventParameter(const char* event, const char* parameter, EventParameterInfo* info);  // 0x40890

// One value of an enumerated property with its label and help text. Names
// not in the reference map.
struct EnumValueDesc {
    int mValue;
    String mName;
    String mDescription;
};

static_assert(sizeof(EnumValueDesc) == 40);

// The PlayArgs::Route values with their descriptions, for the property
// editors that construct the descriptor at 0x2ADB0 and 0x2FF50. At 0x408C0.
// Name not in the reference map.
eastl::vector<EnumValueDesc> GetPlayArgsRouteValues();
