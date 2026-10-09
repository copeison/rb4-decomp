#pragma once

#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/system/Audio.h"
#include "utl/containers/Map.h"
#include "utl/text/Symbol.h"
#include "os/threading/CritSec.h"

class FusionVoicePool;

// Client run by a render target before each mix. The map emits the
// destructor inline in the SoundManager object; this build keeps one copy at
// 0xC09D0. The vtable is at 0x18E5B30.
class FmodPremixCallback {
public:
    virtual ~FmodPremixCallback() {}  // slots 0-1: 0xC09D0, 0xC0A40
    // Slot 2. The map has ExecutePremix(int, unsigned long).
    virtual void ExecutePremix(int numSamples, unsigned long mixCount) = 0;

    LinkedListSizeTracked::Node mCallbackNode;
};

static_assert(offsetof(FmodPremixCallback, mCallbackNode) == 8);
static_assert(sizeof(FmodPremixCallback) == 32);

// Mixer that feeds 128-sample blocks to its AudioBusCallable clients. New
// callables are queued and promoted at the start of each mix. The vtable is
// at 0x19A00E0; the object is 128 bytes. Name not in the reference map.
class AudioMixer : public FmodPremixCallback {
public:
    using CallableList = LinkedListSizeTracked::
        List<AudioBusCallable, &AudioBusCallable::_mCallbackNode>;

    static constexpr int kBlockSamples = 128;  // Name not in the reference map.

    // Inlined into AudioRenderTarget's constructor at 0x1127FC0; an unused
    // copy is at 0x1127770.
    AudioMixer();
    ~AudioMixer() override;  // slots 0-1: 0x1127BA0, 0x1127D80
    // Slot 2 at 0x1127880. Mixes the buffer in 128-sample blocks.
    void ExecutePremix(int numSamples, unsigned long mixCount) override;

    // Enters and leaves mCritSec, as the generators inline them. Names not in
    // the reference map.
    void Lock() {
        mCritSec.Enter();
    }
    void Unlock() {
        mCritSec.Exit();
    }

    // Unlinks a callable from whichever list holds it; false when neither
    // does. At 0x57560. Name not in the reference map.
    bool RemoveCallable(AudioBusCallable* callable);
    // Queues a callable for the next mix. Inlined, for example at 0x267205.
    // Name not in the reference map.
    void AddCallable(AudioBusCallable* callable) {
        ScopedCritSecPtr tracker(&mPendingCritSec);
        mPendingCallables.PushBack(*callable);
    }

    // At 0x1127B90, called by FModSystem. Name not in the reference map.
    void SetSampleRate(int sampleRate);
    // An identical setter at 0x1127B80, called by
    // RecordingAudioRenderTarget. Name not in the reference map.
    void SetOutputSampleRate(int sampleRate);

    // Field names are not in the reference map.
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
// recording targets derive from it. The vtable is at 0x19A0108; the object
// is 280 bytes. The map has no object for it. Name not in the reference
// map.
class AudioRenderTarget {
public:
    // mType values. Names not in the reference map.
    enum Type : int {
        kTypeFmodSystem = 1,
        kTypeRecording = 2,
    };

    AudioRenderTarget(Symbol name, int sampleRate);  // 0x1127FC0
    virtual ~AudioRenderTarget();  // slots 0-1: 0x11281F0, 0x1128330

    // Names below are not in the reference map unless noted.
    virtual int SuspendMixer() = 0;  // slot 2
    virtual int ResumeMixer() = 0;   // slot 3
    // Slot 4 at 0x11284A0: the CPU timer registered under the key, or null.
    virtual void* GetTimer(unsigned long key);
    // Slot 5 at 0x11285D0: replaces the voice pool.
    virtual void InitVoicePool(int hardVoiceLimit, int softVoiceLimit, int numPitchShifters, bool skipVoiceDecoders);
    // Slot 6 at 0x11286A0.
    virtual void ConfigureVoicePool(int softVoiceLimit, int hardVoiceLimit);
    // Slots 7-9 share their code with FModSystem's GetVoicePool at 0x278C40
    // and FmodRecordingAudioRenderTarget's TryBeginMix and EndMix at
    // 0x276180 and 0x276190.
    virtual FusionVoicePool* GetVoicePool();
    virtual bool TryBeginMix();
    virtual bool EndMix();
    virtual void Lock();           // slot 10 at 0x1128350
    virtual void Unlock();         // slot 11 at 0x1128370
    // Slot 12 at 0x11287B0. The binary returns no value here.
    virtual int Update();
    // Slots 13-14 at 0x1128380 and 0x11283F0. The map names these on
    // FModSystem.
    virtual void AddPremixCallback(FmodPremixCallback* callback);
    virtual void RemovePremixCallback(FmodPremixCallback* callback);
    // Slot 15 at 0x11287C0.
    virtual void ExecutePremixCallbacks(unsigned long mixCount);
    // Slot 16, sharing FModSystem's GetMixer at 0x278C50.
    virtual AudioMixer* GetMixer();

    // Names the target and copies its sample rate to the mixer. At
    // 0x11281D0.
    void InitBase(Symbol name);
    // The speaker count of a speaker configuration, or zero. At 0x1128480.
    int GetNumSpeakers(int speakerConfig);
    // Registers a CPU timer under the key. At 0x11286E0.
    void SetTimer(unsigned long key, void* timer);

    Symbol mName;
    Type mType;
    AudioMixer mMixer;
    FusionVoicePool* mVoicePool;
    CritSec mPremixCritSec;
    LinkedListSizeTracked::ListBase mPremixCallbacks;
    int mSampleRate;
    int mBufferSize;
    int mNumBuffers;
    int mMaxSoftwareChannels;  // The FMOD recording target passes its channel count here.
    int mNumRawSpeakers;
    int mUnusedWord;  // No recovered target reads or writes it.
    eastl::map<unsigned long, void*> mTimers;  // CPU timers keyed by source id.
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

// Engine registry of named render targets at 0x19C90B0, kept with the sound
// manager's code. The default target answers the empty name. Names not in
// the reference map.
class AudioRenderTargetRegistry {
public:
    ~AudioRenderTargetRegistry();  // 0xC0F70

    // Deletes every target except the default one under the empty name,
    // then empties the map. At 0xC0F90.
    void DeleteAll();
    // Makes the target the default and registers it under the empty name.
    // At 0xC13E0.
    void SetDefault(AudioRenderTarget* target);
    // Falls back to the default target when the name is unknown and the flag
    // is set. At 0xC14B0.
    AudioRenderTarget* Find(Symbol name, bool useDefault);
    // False when a target with the name is already registered. At 0xC15E0.
    bool Register(AudioRenderTarget* target);
    // False when no target has the name. At 0xC1700.
    bool Unregister(AudioRenderTarget* target);

    eastl::map<Symbol, AudioRenderTarget*> mTargets;
    AudioRenderTarget* mDefault;  // 0x19C90E8
};

static_assert(offsetof(AudioRenderTargetRegistry, mDefault) == 56);

extern AudioRenderTargetRegistry gAudioRenderTargets;
