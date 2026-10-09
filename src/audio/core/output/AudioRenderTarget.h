#pragma once

#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/system/Audio.h"
#include "utl/text/Symbol.h"
#include "os/threading/CritSec.h"

class FmodPremixCallback;

// Mixer that feeds 128-sample blocks to its AudioBusCallable clients. New
// callables are queued and promoted at the start of each mix. The vtable is
// at 0x19A00E0; the object is 128 bytes. Name not in the reference map.
class AudioMixer {
public:
    using CallableList = LinkedListSizeTracked::
        List<AudioBusCallable, &AudioBusCallable::_mCallbackNode>;

    static constexpr int kBlockSamples = 128;  // Name not in the reference map.

    virtual ~AudioMixer();  // slots 0-1: 0x1127BA0, 0x1127D80
    // Slot 2 at 0x1127880. The sample rate and last-buffer arguments are
    // unused; each block takes the rate from mSampleRate. Names not in the
    // reference map.
    virtual void Mix(float sampleRate, unsigned int numSamples, unsigned int mixCount, bool last);

    // Enters and leaves mCritSec, as the generators inline them. Names not in
    // the reference map.
    void Lock() {
        mCritSec.Enter();
    }
    void Unlock() {
        mCritSec.Exit();
    }

    // Unlinks a callable from either list. At 0x57560. Name not in the
    // reference map.
    void RemoveCallable(AudioBusCallable* callable);
    // Queues a callable for the next mix. Inlined, for example at 0x267205.
    // Name not in the reference map.
    void AddCallable(AudioBusCallable* callable) {
        ScopedCritSecPtr tracker(&mPendingCritSec);
        mPendingCallables.PushBack(*callable);
    }

    // At 0x1127B90. Name not in the reference map.
    void SetSampleRate(int sampleRate);

    // Field names are not in the reference map.
    LinkedListSizeTracked::ListBase mUnknown8;
    CritSec mCritSec;
    CallableList mCallables;
    CritSec mPendingCritSec;
    CallableList mPendingCallables;
    int mSampleRate;
    int mMixCount;
    int mBlock;
    bool mLastBlock;
};

static_assert(offsetof(AudioMixer, mCritSec) == 32);
static_assert(offsetof(AudioMixer, mCallables) == 48);
static_assert(offsetof(AudioMixer, mPendingCritSec) == 72);
static_assert(offsetof(AudioMixer, mPendingCallables) == 88);
static_assert(offsetof(AudioMixer, mSampleRate) == 112);
static_assert(offsetof(AudioMixer, mLastBlock) == 124);
static_assert(sizeof(AudioMixer) == 128);

// Named audio output registered with the engine. FModSystem and the
// recording targets derive from it. The vtable is at 0x19A0108 and the
// constructor at 0x1127FC0. Name not in the reference map.
class AudioRenderTarget {
public:
    // mType values. Names not in the reference map.
    enum Type : int {
        kTypeFmodSystem = 1,
        kTypeRecording = 2,
    };

    AudioRenderTarget(Symbol name, int sampleRate);
    virtual ~AudioRenderTarget();  // slots 0-1: 0x11281F0, 0x1128330

    // Names below are not in the reference map unless noted.
    virtual int SuspendMixer() = 0;  // slot 2
    virtual int ResumeMixer() = 0;   // slot 3
    // Slot 4 at 0x11284A0: looks up a CPU timer by key in the map at +224.
    virtual void* GetTimer(unsigned long key);
    // Slot 5 at 0x11285D0.
    virtual void InitVoicePool(int maxVoices, int numBuffers, int unknown, bool unknownFlag);
    // Slot 6 at 0x11286A0.
    virtual void ConfigureVoicePool(int numBuffers, int maxVoices);
    virtual void* GetVoicePool();  // slot 7 at 0x278C40
    virtual bool TryBeginMix();    // slot 8 at 0x276180
    virtual bool EndMix();         // slot 9 at 0x276190
    virtual void Lock();           // slot 10 at 0x1128350
    virtual void Unlock();         // slot 11 at 0x1128370
    virtual int Update();          // slot 12 at 0x11287B0
    // Slots 13-14 at 0x1128380 and 0x11283F0. The map names these on
    // FModSystem.
    virtual void AddPremixCallback(FmodPremixCallback* callback);
    virtual void RemovePremixCallback(FmodPremixCallback* callback);
    // Slot 15 at 0x11287C0.
    virtual void ExecutePremixCallbacks(unsigned long mixCount);
    virtual AudioMixer* GetMixer();  // slot 16 at 0x278C50

    // Names the target and copies its sample rate to the mixer. At
    // 0x11281D0. Name not in the reference map.
    void InitBase(Symbol name);

    Symbol mName;
    Type mType;
    AudioMixer mMixer;
    void* mVoicePool;
    CritSec mPremixCritSec;
    LinkedListSizeTracked::ListBase mPremixCallbacks;
    int mSampleRate;
    int mBufferSize;
    int mNumBuffers;
    int mUnknown212;
    int mNumRawSpeakers;
    int mUnknown220;
    // EASTL map of CPU timers keyed by source id.
    unsigned char mTimers[56];
};

static_assert(offsetof(AudioRenderTarget, mType) == 16);
static_assert(offsetof(AudioRenderTarget, mMixer) == 24);
static_assert(offsetof(AudioRenderTarget, mVoicePool) == 152);
static_assert(offsetof(AudioRenderTarget, mPremixCritSec) == 160);
static_assert(offsetof(AudioRenderTarget, mPremixCallbacks) == 176);
static_assert(offsetof(AudioRenderTarget, mSampleRate) == 200);
static_assert(offsetof(AudioRenderTarget, mBufferSize) == 204);
static_assert(offsetof(AudioRenderTarget, mNumRawSpeakers) == 216);
static_assert(offsetof(AudioRenderTarget, mTimers) == 224);
static_assert(sizeof(AudioRenderTarget) == 280);

// Engine registry of named render targets at 0x19C90B0. Names not in the
// reference map.
class AudioRenderTargetRegistry {
public:
    void Register(AudioRenderTarget* target);    // 0xC15E0
    void Unregister(AudioRenderTarget* target);  // 0xC1700
    // Falls back to the default target when the name is unknown and the flag
    // is set. At 0xC14B0.
    AudioRenderTarget* Find(Symbol name, bool useDefault);
};

extern AudioRenderTargetRegistry gAudioRenderTargets;
// The default target at 0x19C90E8, used when a request names none.
extern AudioRenderTarget* gDefaultAudioRenderTarget;
