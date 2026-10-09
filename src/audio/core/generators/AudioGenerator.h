#pragma once

#include <atomic>
#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/system/Audio.h"
#include "utl/text/Symbol.h"

class AudioEmitterCom;
class AudioGeneratorManager;
class AudioRenderTarget;
class Transform;

// Playback request shared by every generator manager. Only the fields read
// by the FMOD generators are named; the inlined builder in
// AudioGeneratorManager::Play(Symbol, AudioEmitterCom*, bool) at 0x40570
// sets the defaults. Field names are not in the reference map.
struct PlayArgs {
    struct ParameterValue {
        Symbol mName;
        float mValue;
    };

    // EASTL vector view: begin, end, capacity and allocator.
    struct ParameterList {
        ParameterValue* mBegin;
        ParameterValue* mEnd;
        ParameterValue* mCapacity;
        void* mAllocator;
    };

    enum Route : int {
        kRouteDefault = 0,
        kRouteEvent = 1,
        kRouteBus = 2,
    };

    Symbol mName;
    AudioEmitterCom* mEmitter;
    bool mStartPaused;
    bool mStartMuted;
    unsigned char mUnknown18[2];
    bool mHasInitialGain;
    float mInitialGainDb;
    float mInitialGainFadeSecs;
    int mInitialGainPostFade;
    int mUnknown36;
    int mRoute;
    const char* mRoutePath;
    float mSpread;
    Symbol mRenderTarget;
    Symbol mUnknown72;
    Symbol mUnknown80;
    ParameterList* mParameters;
    int mFormat;
    bool mOwnsParameters;
    bool mStreaming;
    unsigned char mUnknown102[10];
    // Optional std::function dialog sink and its context, copied by
    // DialogGenerator::Setup at 0x1127330.
    unsigned char mDialogSink[48];
    void* mDialogContext;
};

static_assert(offsetof(PlayArgs, mStartPaused) == 16);
static_assert(offsetof(PlayArgs, mHasInitialGain) == 20);
static_assert(offsetof(PlayArgs, mInitialGainDb) == 24);
static_assert(offsetof(PlayArgs, mInitialGainPostFade) == 32);
static_assert(offsetof(PlayArgs, mRoute) == 40);
static_assert(offsetof(PlayArgs, mRoutePath) == 48);
static_assert(offsetof(PlayArgs, mSpread) == 56);
static_assert(offsetof(PlayArgs, mRenderTarget) == 64);
static_assert(offsetof(PlayArgs, mParameters) == 88);
static_assert(offsetof(PlayArgs, mFormat) == 96);
static_assert(offsetof(PlayArgs, mStreaming) == 101);
static_assert(offsetof(PlayArgs, mDialogSink) == 112);
static_assert(offsetof(PlayArgs, mDialogContext) == 160);

// Scene component that owns sounds. Only the virtual slots called by the
// audio generators are declared; the earlier slots are placeholders so the
// calls use the recovered vtable offsets. Names not in the reference map.
class AudioEmitterCom {
public:
    virtual void Unknown0();
    virtual void Unknown1();
    virtual void Unknown2();
    virtual void Unknown3();
    virtual void Unknown4();
    virtual void Unknown5();
    virtual void Unknown6();
    virtual void Unknown7();
    virtual void Unknown8();
    virtual void Unknown9();
    virtual void Unknown10();
    virtual void Unknown11();
    virtual void Unknown12();
    virtual void Unknown13();
    virtual void Unknown14();
    virtual void Unknown15();
    // Slot 16: the emitter's mix group, or null. When present, the FMOD
    // generators route through its channel group, whose getter is inlined
    // as null in this build.
    virtual void* GetMixGroup();
    // Slot 17: the emitter's world transform.
    virtual const Transform& GetWorldXfm();
    virtual void Unknown18();
    virtual void Unknown19();
    virtual void Unknown20();
    virtual void Unknown21();
    virtual void Unknown22();
    virtual void Unknown23();
    virtual void Unknown24();
    // Slot 25: whether the emitter is positional.
    virtual bool Is3D();
};

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
    // Inlined into every pool constructor, for example at 0x26A690.
    AudioGenerator()
        : mUnknown8(""),
          mManager(nullptr),
          mIndex(0),
          mState(kStateStopped),
          mRefCount(0),
          mHandle(0xFFFFFFFF),
          mEmitter(nullptr) {
        mPoolNode.InitUnlinked();
    }

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
    virtual bool Unknown16();                         // slot 16: 0xE480. Name not in the reference map.
    virtual bool Unknown17();                         // slot 17: 0xE490. Name not in the reference map.
    virtual bool IsDialog();                          // slot 18: 0xE4A0. Name not in the reference map.
    virtual void Init(AudioGeneratorManager* manager, int index);  // slot 19: 0xE4B0
    virtual ~AudioGenerator();                        // slots 20-21: 0xE4E0, 0xE550
    virtual bool Poll() = 0;                          // slot 22
    virtual void Release() = 0;                       // slot 23
    virtual void* GetPluginData(const char* name);    // slot 24: 0xE5C0. Name not in the reference map.
    virtual void Unknown25();                         // slot 25: 0xE5D0. Name not in the reference map.
    virtual float Unknown26();                        // slot 26: 0xE5E0. Name not in the reference map.
    virtual float Unknown27();                        // slot 27: 0xE5F0. Name not in the reference map.
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
    // Tells the emitter that an HMX DSP plugin bound to this generator. At
    // 0x407C0. Name not in the reference map.
    void NotifyPluginAttached();

    // Field names are not in the reference map.
    Symbol mUnknown8;
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

// Owner of one fixed generator pool. The vtable is at 0x18DFF18.
class AudioGeneratorManager {
public:
    using GeneratorList =
        LinkedListSizeTracked::List<AudioGenerator, &AudioGenerator::mPoolNode>;

    virtual AudioGenerator* Play(const PlayArgs& args) = 0;  // slot 0
    virtual AudioGenerator* Play(Symbol name, AudioEmitterCom* emitter, bool paused);  // slot 1: 0x40570
    virtual AudioGenerator* Prepare(Symbol name, AudioEmitterCom* emitter);  // slot 2: 0x406C0
    virtual void Init();                              // slot 3: 0x40500
    virtual bool Destroy();                           // slot 4: 0x40540
    virtual void Unknown5();                          // slot 5: 0xDD20. Name not in the reference map.
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
    virtual ~AudioGeneratorManager();                 // slots 16-17: 0x40AC0, 0x40AD0

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

// The sound manager's default 2D emitter, used when a request names none.
// At 0x5C20; the map has SoundManager::GetDefault2DEmitter() const.
class SoundManager {
public:
    AudioEmitterCom* GetDefault2DEmitter() const;
};

extern SoundManager gSoundManager;  // Name not in the reference map.
