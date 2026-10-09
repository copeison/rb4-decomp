#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <semaphore.h>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/fmod/api/fmod_api.h"
#include "utl/containers/Std.h"
#include "utl/text/Symbol.h"
#include "os/threading/CritSec.h"

class Transform;
class Vector3;

// FMOD 1.10.04 header version passed to Studio::System::create. Name not in
// the reference map.
constexpr unsigned int kFmodHeaderVersion = 0x00011004;

// Converts an engine transform to FMOD's left-handed attributes. The x axis
// is mirrored and the velocity is zero. Reconstructed from eboot.elf at
// 0x27ACB0.
void Convert(const Transform& xfm, FMOD_3D_ATTRIBUTES& attributes);

class AudioGenerator;
class TempoListener;

// User data shared by the engine's HMX.* DSP plugins. The generator that owns
// the event is stored when the plugin is found in the event's channel group.
// Each plugin's create callback (for example at 0x27CC90) builds it at +8 in
// its state. Names not in the reference map.
struct FmodPluginUserData {
    // Always kPluginUserDataMagic.
    unsigned int mMagic;
    void* mPluginData;
    AudioGenerator* mGenerator;
    // The plugin's tempo listener, registered with the generator's emitter
    // when the plugin binds; null for plugins that ignore the tempo.
    TempoListener* mTempoListener;
};

static_assert(offsetof(FmodPluginUserData, mPluginData) == 8);
static_assert(offsetof(FmodPluginUserData, mTempoListener) == 24);

// The marker the plugins store in FmodPluginUserData::mMagic. Name not in
// the reference map.
constexpr unsigned int kPluginUserDataMagic = 0xD00DFACE;

// Finds the DSP with the given name in a channel group and returns its
// plugin data. Reconstructed from eboot.elf at 0x27AD20. Name not in the
// reference map.
void* FindPluginData(FMOD::ChannelControl* channelGroup, const char* name);

// Points every HMX.* DSP in a channel group at the generator that owns the
// event. Inlined into FmodAudioBusGenerator::_EventProgrammerCallback and
// FmodStudioSoundGenerator::_EventCallback. Name not in the reference map.
void BindPluginsToGenerator(FMOD::ChannelControl* channelGroup, AudioGenerator* generator);

// Engine-side Studio bus controls at 0x19B49F0, reached through the pointer
// at 0x19C9088. Its vtable is at 0x18F1008. Names not in the reference map.
class FmodBusInterface {
public:
    virtual ~FmodBusInterface();                 // slots 0-1: 0x279140, 0x279E10
    virtual bool IsBusLoaded(Symbol path);       // slot 2: 0x279730
    virtual bool IsBusKnown(Symbol path);        // slot 3: 0x279790
    virtual bool SetBusPaused(bool paused, Symbol path);  // slot 4: 0x279440
    virtual bool SetMasterPaused(bool paused);   // slot 5: 0x2794B0
};

extern FmodBusInterface* gFmodBusInterface;

// Lock-protected CPU timer reported as a percentage of one audio buffer. The
// vtable is at 0x18F0EF0. Name not in the reference map.
class AudioCpuTimer {
public:
    virtual ~AudioCpuTimer() {}             // slots 0-1: 0x276DD0, 0x278C70
    virtual void Stop();                    // slot 2: 0x278880
    virtual double GetAveragePercent() const;  // slot 3: 0x278C80
    virtual double GetMaxPercent() const;   // slot 4: 0x278CB0

    void Start() {  // Inlined at every timing site.
        mStartCycles = __builtin_ia32_rdtsc();
        mElapsedCycles = 0;
        mRunning = 1;
    }
    void LockStats();    // Inlined spin lock on mStatsLock.
    void UnlockStats();  // Inlined release of mStatsLock.
    void ResetStats() {
        mTotalMs = 0.0;
        mCount = 0;
        mMaxMs = 0.0;
    }

    // Field names are not in the reference map.
    Symbol mName;
    double mTotalMs;
    int mCount;
    double mMaxMs;
    std::atomic<int> mStatsLock;
    unsigned long mStartCycles;
    unsigned long mElapsedCycles;
    int mRunning;
};

static_assert(offsetof(AudioCpuTimer, mTotalMs) == 16);
static_assert(offsetof(AudioCpuTimer, mCount) == 24);
static_assert(offsetof(AudioCpuTimer, mMaxMs) == 32);
static_assert(offsetof(AudioCpuTimer, mStatsLock) == 40);
static_assert(offsetof(AudioCpuTimer, mRunning) == 64);
static_assert(sizeof(AudioCpuTimer) == 72);

// CPU timer that reports the sum over a rolling window of buffers. The vtable
// is at 0x18F0F28. Name not in the reference map.
class AudioRollingCpuTimer : public AudioCpuTimer {
public:
    ~AudioRollingCpuTimer() override;          // slots 0-1: 0x276DB0, 0x278CD0
    void Stop() override;                      // slot 2: 0x278910
    double GetAveragePercent() const override;  // slot 3: 0x278D10
    double GetMaxPercent() const override;     // slot 4: 0x278D50

    // Field names are not in the reference map.
    double mWindowTotalMs;
    int mWindowIndex;
    double* mWindow;
    int mWindowSize;
};

static_assert(offsetof(AudioRollingCpuTimer, mWindowTotalMs) == 72);
static_assert(offsetof(AudioRollingCpuTimer, mWindow) == 88);
static_assert(sizeof(AudioRollingCpuTimer) == 104);

// The engine's FMOD Studio and low-level systems, and the mix bookkeeping
// around FMOD's premix and postmix callbacks. The vtable is at 0x18F0E40;
// the object is 832 bytes.
class FModSystem : public AudioRenderTarget {
public:
    // Channel and DSP pair whose release waits for the next premix. Name not
    // in the reference map.
    struct DeferredRelease {
        FMOD::ChannelControl* mChannel;
        FMOD::DSP* mDSP;
    };

    // EASTL vector of deferred releases. Name not in the reference map.
    struct DeferredReleaseList {
        DeferredRelease* mBegin;
        DeferredRelease* mEnd;
        DeferredRelease* mCapacity;
        void* mAllocator;
    };

    // Premix client that drains the deferred releases. The vtable is at
    // 0x18F0F60. Name not in the reference map.
    class DeferredReleaser : public FmodPremixCallback {
    public:
        ~DeferredReleaser() override;  // slots 0-1: 0x276D40, 0x278D80
        // Slot 2 at 0x278DF0.
        void ExecutePremix(int numSamples, unsigned long mixCount) override;

        FModSystem* mSystem;
    };

    // The map has FModSystem(bool).
    FModSystem();                 // 0x276530
    ~FModSystem() override;       // slots 0-1: 0x276A40, 0x276DE0

    int SuspendMixer() override;  // slot 2: 0x277B20
    int ResumeMixer() override;   // slot 3: 0x277B30
    FusionVoicePool* GetVoicePool() override;  // slot 7: 0x278C40
    bool TryBeginMix() override;  // slot 8: 0x277B40
    bool EndMix() override;       // slot 9: 0x277B80
    int Update() override;        // slot 12: 0x2780D0. The map has Poll().
    // Slot 15 at 0x2781C0.
    void ExecutePremixCallbacks(unsigned long mixCount) override;
    AudioMixer* GetMixer() override;   // slot 16: 0x278C50
    // Slot 17 at 0x276F30. The map has Init(bool).
    virtual int Init(
        Symbol name,
        bool unknown,
        FMOD_OUTPUTTYPE output,
        int bufferLength,
        int numBuffers,
        int softwareChannels,
        int maxChannels,
        bool initFmod);
    // Slot 18 at 0x277760: adopts a Studio system created elsewhere. Name
    // not in the reference map.
    virtual int InitWithStudioSystem(
        Symbol name,
        FMOD::Studio::System* system,
        int bufferLength,
        int numBuffers,
        int sampleRate);
    // Slot 19 at 0x278C60. Name not in the reference map.
    virtual int GetClassId();

    // Reconstructed from eboot.elf at 0x2773C0. The map has InitFmod(bool).
    FMOD_RESULT InitFmod(bool unknown, FMOD_OUTPUTTYPE output, int softwareChannels);
    // Reconstructed from eboot.elf at 0x2786D0. Name not in the reference map.
    int InitBufferedOutput();
    // Reconstructed from eboot.elf at 0x278270.
    void RegisterPlugins();
    // Reconstructed from eboot.elf at 0x277A80.
    void Terminate();
    // Reconstructed from eboot.elf at 0x277840. Name not in the reference
    // map.
    void SetStudioSystem(FMOD::Studio::System* system);
    // Reconstructed from eboot.elf at 0x2783A0. Name not in the reference
    // map.
    void SetSpeakerConfig(int config);
    // Reconstructed from eboot.elf at 0x276FD0. Name not in the reference
    // map.
    void _InitTimers(int bufferSetWindow);
    // Reconstructed from eboot.elf at 0x278A00. Name not in the reference
    // map.
    void DeferRelease(FMOD::ChannelControl* channel, FMOD::DSP* dsp);
    // Reconstructed from eboot.elf at 0x277BA0. The map has GetCPUPercent();
    // this build reports each timer's average and maximum.
    void GetCPUPercent(
        double* hmxAverage,
        double* hmxMax,
        double* fmodAverage,
        double* fmodMax,
        double* unused0,
        double* unused1,
        double* unused2,
        double* unused3,
        double* bufferSetAverage,
        double* bufferSetMax,
        void* sourceAverages,
        void* sourceMaxes,
        void* sourceKeys);

    // Reconstructed from eboot.elf at 0x2783E0.
    static FMOD_RESULT _SystemCallback(
        FMOD_SYSTEM* system,
        FMOD_SYSTEM_CALLBACK_TYPE type,
        void* commandData1,
        void* commandData2,
        void* userData);

    static FModSystem* Get() {
        return sSystem;
    }
    FMOD::Studio::System* GetStudioSystem() const {
        return mStudioSystem;
    }
    FMOD::System* GetLowLevelSystem() const {
        return mLowLevelSystem;
    }

    // The engine's primary system at 0x19F29D8. Name not in the reference
    // map.
    static FModSystem* sSystem;

    // Field names are not in the reference map.
    FMOD::Studio::System* mStudioSystem;
    FMOD::System* mLowLevelSystem;
    unsigned int mDspBufferLength;
    FMOD_SPEAKERMODE mSpeakerMode;
    int mMaxChannels;
    bool mShuttingDown;
    std::function<FMOD_RESULT(FMOD_OUTPUT_STATE*)> mBufferedOutputCallback;
    // The recording target that owns this system; cleared by the
    // constructor (0x276530) and set only by FmodRecordingAudioRenderTarget.
    // Nothing in this build reads it, so the name rests on that store.
    AudioRenderTarget* mOwnerTarget;
    unsigned long mMixCount;
    AudioCpuTimer mHmxTimer;
    AudioCpuTimer mFmodTimer;
    AudioRollingCpuTimer mBufferSetTimer;
    // EASTL list of per-source AudioCpuTimer entries.
    LinkedListSizeTracked::ListBase mSourceTimers;
    void* mSourceTimersAllocator;
    // Set to -1 by the constructor and never read in this build; the name
    // only records the invalid-handle value. The evidence is weak.
    long mUnusedHandle;
    bool mInMix;
    // Eight-byte aligned in the binary; bytes 673-679 are never accessed.
    alignas(8) sem_t mMixSemaphore;
    bool mUpdateStudio;
    CritSec mDeferredCritSec;
    DeferredReleaseList mDeferredReleases[2];
    int mDeferredBuffer;
    DeferredReleaser mDeferredReleaser;
};

static_assert(offsetof(FModSystem, mStudioSystem) == 280);
static_assert(offsetof(FModSystem, mLowLevelSystem) == 288);
static_assert(offsetof(FModSystem, mDspBufferLength) == 296);
static_assert(offsetof(FModSystem, mSpeakerMode) == 300);
static_assert(offsetof(FModSystem, mMaxChannels) == 304);
static_assert(offsetof(FModSystem, mShuttingDown) == 308);
static_assert(offsetof(FModSystem, mBufferedOutputCallback) == 320);
static_assert(offsetof(FModSystem, mMixCount) == 376);
static_assert(offsetof(FModSystem, mHmxTimer) == 384);
static_assert(offsetof(FModSystem, mFmodTimer) == 456);
static_assert(offsetof(FModSystem, mBufferSetTimer) == 528);
static_assert(offsetof(FModSystem, mSourceTimers) == 632);
static_assert(offsetof(FModSystem, mInMix) == 672);
static_assert(offsetof(FModSystem, mMixSemaphore) == 680);
static_assert(offsetof(FModSystem, mUpdateStudio) == 696);
static_assert(offsetof(FModSystem, mDeferredCritSec) == 704);
static_assert(offsetof(FModSystem, mDeferredReleases) == 720);
static_assert(offsetof(FModSystem, mDeferredBuffer) == 784);
static_assert(offsetof(FModSystem, mDeferredReleaser) == 792);
static_assert(sizeof(FModSystem) == 832);
