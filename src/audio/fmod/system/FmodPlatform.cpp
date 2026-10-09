#include "audio/fmod/system/FmodPlatform.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "audio/fmod/io/FmodFileWrapper.h"
#include "audio/fmod/mixing/AnalysisPlugin.h"
#include "audio/fmod/mixing/BitCrusherPlugin.h"
#include "audio/fmod/mixing/DelayPlugin.h"
#include "audio/fmod/mixing/FilterPlugin.h"
#include "audio/fmod/mixing/FmodGain.h"
#include "audio/fmod/mixing/SignalTapPlugin.h"
#include "audio/fmod/mixing/SmbPitchShiftPlugin.h"
#include "audio/fmod/mixing/SoundClashSlotPlugin.h"
#include "audio/fmod/mixing/StutterPlugin.h"
#include "audio/fmod/mixing/TremoloPlugin.h"
#include "audio/fmod/mixing/VibePlugin.h"
#include "audio/fmod/mixing/WahPlugin.h"
#include "audio/core/generators/AudioGenerator.h"
#include "math/transform/Transform.h"
#include "utl/time/Timer.h"
#include "os/threading/CritSec.h"

FModSystem* FModSystem::sSystem;

namespace {

constexpr double kPercent = 100.0;

FMOD_VECTOR ConvertVector(const Vector3& v) {
    return {-v.x, v.y, v.z};
}

// Waits on the mix semaphore, retrying when a signal interrupts the wait.
void WaitForMix(sem_t& semaphore) {
    while (sem_wait(&semaphore) != 0) {
        (void)errno;
    }
}

// HMX.BufferedOutput, the FMOD output plugin of the recording target. The
// callbacks are at 0x2763F0 through 0x276520.
constexpr unsigned int kOutputPluginApiVersion = 3;
constexpr int kBufferedOutputDrivers = 128;

FMOD_RESULT BufferedOutputGetNumDrivers(FMOD_OUTPUT_STATE*, int* numDrivers) {
    *numDrivers = kBufferedOutputDrivers;
    return FMOD_OK;
}

FMOD_RESULT BufferedOutputGetDriverInfo(
    FMOD_OUTPUT_STATE*,
    int id,
    char* name,
    int nameLength,
    FMOD_GUID*,
    int* systemRate,
    FMOD_SPEAKERMODE* speakerMode,
    int* speakerModeChannels) {
    if (name != nullptr) {
        snprintf(name, nameLength, "null_output_%d", id);
        name[nameLength - 1] = '\0';
    }
    *systemRate = static_cast<int>(Audio::GetSamplesPerSecond());
    *speakerMode = FMOD_SPEAKERMODE_STEREO;
    *speakerModeChannels = 2;
    return FMOD_OK;
}

FMOD_RESULT BufferedOutputInit(
    FMOD_OUTPUT_STATE* state,
    int,
    FMOD_INITFLAGS,
    int*,
    FMOD_SPEAKERMODE* speakerMode,
    int* speakerModeChannels,
    FMOD_SOUND_FORMAT* outputFormat,
    int,
    int,
    void* extraDriverData) {
    auto* system = static_cast<FModSystem*>(extraDriverData);
    state->plugindata = system;
    *outputFormat = FMOD_SOUND_FORMAT_PCMFLOAT;
    *speakerMode = system->mSpeakerMode;
    *speakerModeChannels = system->mNumRawSpeakers;
    return FMOD_OK;
}

FMOD_RESULT BufferedOutputClose(FMOD_OUTPUT_STATE*) {
    return FMOD_OK;
}

FMOD_RESULT BufferedOutputUpdate(FMOD_OUTPUT_STATE* state) {
    auto* system = static_cast<FModSystem*>(state->plugindata);
    return system->mBufferedOutputCallback(state);
}

FMOD_RESULT BufferedOutputGetHandle(FMOD_OUTPUT_STATE*, void**) {
    return FMOD_OK;
}

// At 0x19B4A80. Name not in the reference map.
const FMOD_OUTPUT_DESCRIPTION gBufferedOutputDescription = {
    kOutputPluginApiVersion,
    "HMX.BufferedOutput",
    1,
    FMOD_OUTPUT_METHOD_MIX_DIRECT,
    BufferedOutputGetNumDrivers,
    BufferedOutputGetDriverInfo,
    BufferedOutputInit,
    nullptr,
    nullptr,
    BufferedOutputClose,
    BufferedOutputUpdate,
    BufferedOutputGetHandle,
    {},
};

// Speaker configuration tables at 0x125D1F0 and 0x125D200.
constexpr FMOD_SPEAKERMODE kSpeakerModes[] = {
    FMOD_SPEAKERMODE_MONO,
    FMOD_SPEAKERMODE_STEREO,
    FMOD_SPEAKERMODE_5POINT1,
    FMOD_SPEAKERMODE_7POINT1,
};
constexpr int kRawSpeakerCounts[] = {1, 2, 6, 8};

constexpr FMOD_SYSTEM_CALLBACK_TYPE kMixCallbacks =
    FMOD_SYSTEM_CALLBACK_PREMIX | FMOD_SYSTEM_CALLBACK_POSTMIX;

}  // namespace

// Reconstructed from eboot.elf at 0x27ACB0.
void Convert(const Transform& xfm, FMOD_3D_ATTRIBUTES& attributes) {
    attributes.position = ConvertVector(xfm.v);
    attributes.velocity = {-0.0F, 0.0F, 0.0F};
    attributes.forward = ConvertVector(xfm.m.y);
    attributes.up = ConvertVector(xfm.m.z);
}

// Reconstructed from eboot.elf at 0x27AD20.
void* FindPluginData(FMOD::ChannelControl* channelGroup, const char* name) {
    int numDSPs = 0;
    channelGroup->getNumDSPs(&numDSPs);
    for (int index = 0; index < numDSPs; ++index) {
        FMOD::DSP* dsp = nullptr;
        channelGroup->getDSP(index, &dsp);
        char dspName[256];
        dsp->getInfo(dspName, nullptr, nullptr, nullptr, nullptr);
        if (std::strcmp(name, dspName) == 0) {
            FmodPluginUserData* data = nullptr;
            dsp->getUserData(reinterpret_cast<void**>(&data));
            if (data != nullptr) {
                return data->mPluginData;
            }
        }
    }
    return nullptr;
}

// Inlined at 0x267410 and 0x26FFE0. The original compares the DSP name's
// first four characters through a temporary String.
void BindPluginsToGenerator(FMOD::ChannelControl* channelGroup, AudioGenerator* generator) {
    int numDSPs = 0;
    channelGroup->getNumDSPs(&numDSPs);
    for (int index = 0; index < numDSPs; ++index) {
        FMOD::DSP* dsp = nullptr;
        channelGroup->getDSP(index, &dsp);
        char name[256];
        dsp->getInfo(name, nullptr, nullptr, nullptr, nullptr);
        if (std::strncmp(name, "HMX.", 4) != 0) {
            continue;
        }
        FmodPluginUserData* data = nullptr;
        dsp->getUserData(reinterpret_cast<void**>(&data));
        if (data != nullptr) {
            data->mGenerator = generator;
            if (data->mTempoListener != nullptr) {
                generator->RegisterTempoListener(data->mTempoListener);
            }
        }
    }
}

// Inlined spin lock, for example at 0x278880.
void AudioCpuTimer::LockStats() {
    int expected = 0;
    while (!mStatsLock.compare_exchange_weak(expected, 1)) {
        expected = 0;
    }
}

void AudioCpuTimer::UnlockStats() {
    int expected = 1;
    while (!mStatsLock.compare_exchange_weak(expected, 0)) {
        expected = 1;
    }
}

// Reconstructed from eboot.elf at 0x278880.
void AudioCpuTimer::Stop() {
    if (mRunning > 0 && --mRunning == 0) {
        mElapsedCycles += Hmx::Timer::GetCycleCounter() - mStartCycles;
    }
    const double ms = Hmx::Timer::CyclesToMs(mElapsedCycles);
    LockStats();
    mTotalMs += ms;
    ++mCount;
    if (ms > mMaxMs) {
        mMaxMs = ms;
    }
    UnlockStats();
}

// Reconstructed from eboot.elf at 0x278C80.
double AudioCpuTimer::GetAveragePercent() const {
    const double average = mCount != 0 ? mTotalMs / mCount : 0.0;
    return average / Audio::sMsPerBuffer * kPercent;
}

// Reconstructed from eboot.elf at 0x278CB0.
double AudioCpuTimer::GetMaxPercent() const {
    return mMaxMs / Audio::sMsPerBuffer * kPercent;
}

// Reconstructed from eboot.elf at 0x278910. Each sample is the sum of the
// last mWindowSize buffers.
void AudioRollingCpuTimer::Stop() {
    if (mRunning > 0 && --mRunning == 0) {
        mElapsedCycles += Hmx::Timer::GetCycleCounter() - mStartCycles;
    }
    const double ms = Hmx::Timer::CyclesToMs(mElapsedCycles);
    LockStats();
    const int slot = mWindowIndex % mWindowSize;
    mWindowTotalMs += ms - mWindow[slot];
    mWindow[slot] = ms;
    mTotalMs += mWindowTotalMs;
    ++mCount;
    ++mWindowIndex;
    if (mWindowTotalMs > mMaxMs) {
        mMaxMs = mWindowTotalMs;
    }
    UnlockStats();
}

// Reconstructed from eboot.elf at 0x278D10.
double AudioRollingCpuTimer::GetAveragePercent() const {
    const double average = mCount != 0 ? mTotalMs / mCount : 0.0;
    return average / (mWindowSize * Audio::sMsPerBuffer) * kPercent;
}

// Reconstructed from eboot.elf at 0x278D50.
double AudioRollingCpuTimer::GetMaxPercent() const {
    return mMaxMs / (mWindowSize * Audio::sMsPerBuffer) * kPercent;
}

// Reconstructed from eboot.elf at 0x276F30.
int FModSystem::Init(
    Symbol name,
    bool,
    FMOD_OUTPUTTYPE output,
    int bufferLength,
    int numBuffers,
    int softwareChannels,
    int maxChannels,
    bool initFmod) {
    InitBase(name);
    sem_init(&mMixSemaphore, 0, 1);
    mBufferSize = bufferLength;
    mNumBuffers = numBuffers;
    mMaxChannels = maxChannels;
    _InitTimers(numBuffers);
    int result = 0;
    if (initFmod) {
        result = InitFmod(false, output, softwareChannels);
    }
    AddPremixCallback(&mDeferredReleaser);
    return result;
}

// Reconstructed from eboot.elf at 0x2773C0. A header mismatch only formats
// the diagnostic "header version is %u.%u.%u"; startup continues.
FMOD_RESULT FModSystem::InitFmod(bool, FMOD_OUTPUTTYPE output, int softwareChannels) {
    FMOD::Studio::System::create(&mStudioSystem, kFmodHeaderVersion);
    mStudioSystem->setUserData(this);
    mStudioSystem->getLowLevelSystem(&mLowLevelSystem);
    mLowLevelSystem->setUserData(this);
    if (mLowLevelSystem->setOutput(output) != FMOD_OK) {
        mLowLevelSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
    }

    FMOD_ADVANCEDSETTINGS settings = {};
    settings.cbSize = sizeof(settings);
    mLowLevelSystem->getAdvancedSettings(&settings);
    settings.stackSizeMixer += 16 * 1024;
    settings.commandQueueSize *= 4;
    mLowLevelSystem->setAdvancedSettings(&settings);

    FMOD_STUDIO_ADVANCEDSETTINGS studioSettings = {};
    studioSettings.cbsize = sizeof(studioSettings);
    mStudioSystem->getAdvancedSettings(&studioSettings);
    studioSettings.commandqueuesize *= 4;
    mStudioSystem->setAdvancedSettings(&studioSettings);

    mLowLevelSystem->setSoftwareChannels(softwareChannels);
    const FMOD_RESULT result = mStudioSystem->initialize(
        mMaxChannels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    if (result == FMOD_OK) {
        mLowLevelSystem->setFileSystem(
            FmodFileOpen,
            FmodFileClose,
            FmodFileRead,
            FmodFileSeek,
            FmodFileAsyncRead,
            FmodFileAsyncCancel,
            -1);
        int driver = 0;
        char driverName[256];
        mLowLevelSystem->getDriver(&driver);
        mLowLevelSystem->getDriverInfo(
            driver, driverName, sizeof(driverName), nullptr, &mSampleRate, nullptr, nullptr);
        FMOD_SPEAKERMODE speakerMode;
        int rawSpeakers;
        mLowLevelSystem->getSoftwareFormat(&mSampleRate, &speakerMode, &rawSpeakers);
        int numBuffers;
        mLowLevelSystem->getDSPBufferSize(&mDspBufferLength, &numBuffers);
        Audio::SetAudioSystemProperties(mSampleRate, mDspBufferLength);
        mMixer.SetSampleRate(mSampleRate);
        RegisterPlugins();
        mLowLevelSystem->setCallback(_SystemCallback, kMixCallbacks);
    }
    mStudioSystem->isValid();
    mStudioSystem->update();
    return result;
}

// Reconstructed from eboot.elf at 0x277760.
int FModSystem::InitWithStudioSystem(
    Symbol name,
    FMOD::Studio::System* system,
    int bufferLength,
    int numBuffers,
    int sampleRate) {
    mUpdateStudio = false;
    InitBase(name);
    sem_init(&mMixSemaphore, 0, 1);
    mNumBuffers = numBuffers;
    mBufferSize = bufferLength;
    mSampleRate = sampleRate;
    if (system != nullptr) {
        FMOD::System* lowLevel = nullptr;
        system->getLowLevelSystem(&lowLevel);
        unsigned int systemBufferLength;
        int systemNumBuffers;
        lowLevel->getDSPBufferSize(&systemBufferLength, &systemNumBuffers);
        mBufferSize = systemBufferLength;
        mNumBuffers = systemNumBuffers;
        _InitTimers(systemNumBuffers);
        SetStudioSystem(system);
    }
    AddPremixCallback(&mDeferredReleaser);
    return 0;
}

// Reconstructed from eboot.elf at 0x277840. A null system takes the same
// path as Terminate.
void FModSystem::SetStudioSystem(FMOD::Studio::System* system) {
    if (mStudioSystem == system) {
        return;
    }
    if (system == nullptr) {
        WaitForMix(mMixSemaphore);
        mShuttingDown = true;
        mDeferredCritSec.Enter();
        mDeferredReleases[0].mEnd = mDeferredReleases[0].mBegin;
        mDeferredReleases[1].mEnd = mDeferredReleases[1].mBegin;
        mDeferredCritSec.Exit();
        mStudioSystem = nullptr;
        mLowLevelSystem = nullptr;
        sem_post(&mMixSemaphore);
        return;
    }

    mStudioSystem = system;
    mStudioSystem->getLowLevelSystem(&mLowLevelSystem);
    unsigned int version;
    void* userData;
    mLowLevelSystem->getVersion(&version);
    mStudioSystem->getUserData(&userData);
    mStudioSystem->setUserData(this);
    mLowLevelSystem->getUserData(&userData);
    mLowLevelSystem->setUserData(this);

    int driver = 0;
    char driverName[256];
    mLowLevelSystem->getDriver(&driver);
    mLowLevelSystem->getDriverInfo(
        driver, driverName, sizeof(driverName), nullptr, &mSampleRate, nullptr, nullptr);
    FMOD_SPEAKERMODE speakerMode;
    int rawSpeakers;
    mLowLevelSystem->getSoftwareFormat(&mSampleRate, &speakerMode, &rawSpeakers);
    int numBuffers;
    mLowLevelSystem->getDSPBufferSize(&mDspBufferLength, &numBuffers);
    mMixer.SetSampleRate(mSampleRate);
    RegisterPlugins();
    mShuttingDown = false;
    sem_post(&mMixSemaphore);
    mLowLevelSystem->setCallback(_SystemCallback, kMixCallbacks);
    mStudioSystem->isValid();
    mStudioSystem->update();
}

// Reconstructed from eboot.elf at 0x277A80.
void FModSystem::Terminate() {
    WaitForMix(mMixSemaphore);
    mShuttingDown = true;
    mDeferredCritSec.Enter();
    mDeferredReleases[0].mEnd = mDeferredReleases[0].mBegin;
    mDeferredReleases[1].mEnd = mDeferredReleases[1].mBegin;
    mDeferredCritSec.Exit();
    mStudioSystem = nullptr;
    mLowLevelSystem = nullptr;
    sem_post(&mMixSemaphore);
}

// Reconstructed from eboot.elf at 0x277B20.
int FModSystem::SuspendMixer() {
    return mLowLevelSystem->mixerSuspend();
}

// Reconstructed from eboot.elf at 0x277B30.
int FModSystem::ResumeMixer() {
    return mLowLevelSystem->mixerResume();
}

// Reconstructed from eboot.elf at 0x277B40.
bool FModSystem::TryBeginMix() {
    WaitForMix(mMixSemaphore);
    return true;
}

// Reconstructed from eboot.elf at 0x277B80.
bool FModSystem::EndMix() {
    sem_post(&mMixSemaphore);
    return true;
}

// Reconstructed from eboot.elf at 0x2780D0.
int FModSystem::Update() {
    if (!mUpdateStudio) {
        return FMOD_OK;
    }
    return mStudioSystem->update();
}

// Reconstructed from eboot.elf at 0x2781C0.
void FModSystem::ExecutePremixCallbacks(unsigned long mixCount) {
    {
        ScopedCritSecPtr tracker(&mPremixCritSec);
        auto* const sentinel = mPremixCallbacks.Sentinel();
        for (auto* node = mPremixCallbacks.mNext; node != sentinel; node = node->mNext) {
            auto* callback = reinterpret_cast<FmodPremixCallback*>(
                reinterpret_cast<char*>(node) - offsetof(FmodPremixCallback, mCallbackNode));
            callback->ExecutePremix(mDspBufferLength, mixCount);
        }
    }
    mMixer.ExecutePremix(mDspBufferLength, mixCount);
}

// Reconstructed from eboot.elf at 0x278270.
void FModSystem::RegisterPlugins() {
    mStudioSystem->registerPlugin(AnalysisPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(BitCrusherPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(DelayPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(FilterPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(FmodGainPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(SignalTapPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(SmbPitchShiftPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(StutterPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(SoundClashSlotPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(TremoloPlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(HmxVibePlugin::GetDSPDescription());
    mStudioSystem->registerPlugin(HmxWahPlugin::GetDSPDescription());
}

// Reconstructed from eboot.elf at 0x2783A0.
void FModSystem::SetSpeakerConfig(int config) {
    if (static_cast<unsigned int>(config) <= 3) {
        mSpeakerMode = kSpeakerModes[config];
        mNumRawSpeakers = kRawSpeakerCounts[config];
    }
}

// Reconstructed from eboot.elf at 0x2783E0. The premix callback times the
// engine's premix work and starts the FMOD mix timers; the postmix callback
// closes them and releases the mix semaphore taken by premix.
FMOD_RESULT FModSystem::_SystemCallback(
    FMOD_SYSTEM*,
    FMOD_SYSTEM_CALLBACK_TYPE type,
    void*,
    void*,
    void* userData) {
    auto* system = static_cast<FModSystem*>(userData);
    if (system == nullptr || system->mShuttingDown || system->mStudioSystem == nullptr) {
        return FMOD_OK;
    }

    auto* const sentinel = system->mSourceTimers.Sentinel();
    if (type == FMOD_SYSTEM_CALLBACK_POSTMIX) {
        if (!system->mInMix) {
            return FMOD_OK;
        }
        system->mFmodTimer.AudioCpuTimer::Stop();
        system->mBufferSetTimer.AudioRollingCpuTimer::Stop();
        for (auto* node = system->mSourceTimers.mNext; node != sentinel; node = node->mNext) {
            reinterpret_cast<AudioCpuTimer*>(node + 1)->Stop();
        }
        system->mInMix = false;
        sem_post(&system->mMixSemaphore);
        return FMOD_OK;
    }
    if (type != FMOD_SYSTEM_CALLBACK_PREMIX) {
        return FMOD_OK;
    }

    WaitForMix(system->mMixSemaphore);
    if (system->mStudioSystem == nullptr) {
        sem_post(&system->mMixSemaphore);
        return FMOD_OK;
    }
    system->mInMix = true;
    ++system->mMixCount;
    system->mHmxTimer.Start();
    system->mFmodTimer.Start();
    system->mBufferSetTimer.Start();
    for (auto* node = system->mSourceTimers.mNext; node != sentinel; node = node->mNext) {
        auto* timer = reinterpret_cast<AudioCpuTimer*>(node + 1);
        timer->mElapsedCycles = 0;
        timer->mRunning = 0;
    }
    system->ExecutePremixCallbacks(system->mMixCount);
    system->mHmxTimer.AudioCpuTimer::Stop();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2786D0. Creates a Studio system that
// renders through HMX.BufferedOutput instead of the platform output.
int FModSystem::InitBufferedOutput() {
    FMOD::Studio::System::create(&mStudioSystem, kFmodHeaderVersion);
    mStudioSystem->getLowLevelSystem(&mLowLevelSystem);
    mStudioSystem->setUserData(this);
    mLowLevelSystem->setUserData(this);

    unsigned int output;
    mLowLevelSystem->registerOutput(&gBufferedOutputDescription, &output);
    mLowLevelSystem->setOutputByPlugin(output);
    mLowLevelSystem->setDSPBufferSize(mBufferSize, mNumBuffers);
    mLowLevelSystem->setSoftwareChannels(mMaxChannels);
    mLowLevelSystem->setSoftwareFormat(mSampleRate, mSpeakerMode, mNumRawSpeakers);
    const FMOD_RESULT result = mStudioSystem->initialize(
        mMaxChannels,
        FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE,
        FMOD_INIT_STREAM_FROM_UPDATE | FMOD_INIT_MIX_FROM_UPDATE | FMOD_INIT_3D_RIGHTHANDED,
        this);

    int sampleRate;
    FMOD_SPEAKERMODE speakerMode;
    int rawSpeakers;
    mLowLevelSystem->getSoftwareFormat(&sampleRate, &speakerMode, &rawSpeakers);
    int numBuffers;
    mLowLevelSystem->getDSPBufferSize(&mDspBufferLength, &numBuffers);
    RegisterPlugins();
    if (result != FMOD_OK) {
        return 1;
    }
    mLowLevelSystem->setFileSystem(
        FmodFileOpen,
        FmodFileClose,
        FmodFileRead,
        FmodFileSeek,
        FmodFileAsyncRead,
        FmodFileAsyncCancel,
        -1);
    mLowLevelSystem->setCallback(_SystemCallback, kMixCallbacks);
    return 0;
}

// Reconstructed from eboot.elf at 0x278A00. Releases are dropped once the
// Studio system is gone.
void FModSystem::DeferRelease(FMOD::ChannelControl* channel, FMOD::DSP* dsp) {
    ScopedCritSecPtr tracker(&mDeferredCritSec);
    if (mStudioSystem == nullptr) {
        return;
    }
    auto& list = mDeferredReleases[mDeferredBuffer];
    if (list.mEnd >= list.mCapacity) {
        const unsigned long count = list.mEnd - list.mBegin;
        const unsigned long capacity = count != 0 ? count * 2 : 1;
        auto* storage = static_cast<DeferredRelease*>(HmxAllocator::gStlAllocator.allocate(
            capacity * sizeof(DeferredRelease)));
        for (unsigned long index = 0; index < count; ++index) {
            storage[index] = list.mBegin[index];
        }
        if (list.mBegin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                list.mBegin,
                (list.mCapacity - list.mBegin) * sizeof(DeferredRelease));
        }
        list.mBegin = storage;
        list.mEnd = storage + count;
        list.mCapacity = storage + capacity;
    }
    *list.mEnd++ = {channel, dsp};
}

// Reconstructed from eboot.elf at 0x278DF0. Swaps the double buffer, then
// stops each channel and releases its DSP outside the lock.
void FModSystem::DeferredReleaser::ExecutePremix(int, unsigned long) {
    FModSystem* system = mSystem;
    system->mDeferredCritSec.Enter();
    const int buffer = system->mDeferredBuffer;
    system->mDeferredBuffer = (system->mDeferredBuffer & 1) == 0;
    system->mDeferredCritSec.Exit();

    auto& list = system->mDeferredReleases[buffer];
    for (auto* release = list.mBegin; release != list.mEnd; ++release) {
        FMOD::DSP* dsp = release->mDSP;
        release->mChannel->stop();
        dsp->release();
    }
    list.mEnd = list.mBegin;
}

// Reconstructed from eboot.elf at 0x278C40.
FusionVoicePool* FModSystem::GetVoicePool() {
    return mVoicePool;
}

// Reconstructed from eboot.elf at 0x278C50.
AudioMixer* FModSystem::GetMixer() {
    return &mMixer;
}

// Reconstructed from eboot.elf at 0x278C60.
int FModSystem::GetClassId() {
    return 46;
}
