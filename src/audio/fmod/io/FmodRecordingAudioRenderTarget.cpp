#include "audio/fmod/io/FmodRecordingAudioRenderTarget.h"

namespace {

// Reconstructed from eboot.elf at 0x276110.
int RecordThreadEntry(void* context) {
    static_cast<FmodRecordingAudioRenderTarget*>(context)->RecordLoop();
    return 0;
}

// FMOD_OUTPUTTYPE requested for the recording system. Name not in the
// reference map.
constexpr auto kRecordingOutput = static_cast<FMOD_OUTPUTTYPE>(1);

}  // namespace

// Reconstructed from eboot.elf at 0x275E20. The embedded FMOD system renders
// through HMX.BufferedOutput; its update callback pulls each mixer block into
// the recording buffer.
FmodRecordingAudioRenderTarget::FmodRecordingAudioRenderTarget(
    Symbol name,
    const char* path,
    int bufferLength,
    int maxChannels,
    int sampleRate,
    float gain,
    int speakerConfig)
    : RecordingAudioRenderTarget(
          name, path, bufferLength, maxChannels, sampleRate, gain, speakerConfig) {
    mRecordThread.Init("Unknown Thread!");
    mType = kTypeRecording;
    mFModSystem.Init(
        name, false, kRecordingOutput, bufferLength, 2, maxChannels, maxChannels, false);
    mFModSystem.mOwnerTarget = this;
    mFModSystem.SetSpeakerConfig(speakerConfig);
    mFModSystem.InitBufferedOutput();
    mFModSystem.mBufferedOutputCallback = [this](FMOD_OUTPUT_STATE* state) {
        return state->readfrommixer(state, mMixBuffer, mBufferSize);
    };
    gAudioRenderTargets.Register(this);
}

// Reconstructed from eboot.elf at 0x275FB0.
FmodRecordingAudioRenderTarget::~FmodRecordingAudioRenderTarget() {
    StopRecording();
    gAudioRenderTargets.Unregister(this);
    ReleaseRecording();
    mRecordThread.mThread._ForceKillThread();
}

// Reconstructed from eboot.elf at 0x276080.
void FmodRecordingAudioRenderTarget::StartAsyncRecording() {
    const auto& task = *ThreadMap::GetTaskSettings("async_audio_record");
    mRecordThread.Create(
        RecordThreadEntry,
        this,
        "RecordingAudioRenderTarget",
        task.mProcessor,
        task.mPriority,
        task.mStackSize,
        task.mAffinityMask);
    mRecordThread.mThread.Start();
}

// Reconstructed from eboot.elf at 0x276120.
void FmodRecordingAudioRenderTarget::WaitForRecording() {
    mRecordThread.mThread._Join();
}

// Reconstructed from eboot.elf at 0x276130.
int FmodRecordingAudioRenderTarget::SuspendMixer() {
    return mFModSystem.mLowLevelSystem->mixerSuspend();
}

// Reconstructed from eboot.elf at 0x276140.
int FmodRecordingAudioRenderTarget::ResumeMixer() {
    return mFModSystem.mLowLevelSystem->mixerResume();
}

// Reconstructed from eboot.elf at 0x276150.
void FmodRecordingAudioRenderTarget::InitVoicePool(
    int hardVoiceLimit, int softVoiceLimit, int numPitchShifters, bool skipVoiceDecoders) {
    mFModSystem.AudioRenderTarget::InitVoicePool(
        hardVoiceLimit, softVoiceLimit, numPitchShifters, skipVoiceDecoders);
}

// Reconstructed from eboot.elf at 0x276160.
void FmodRecordingAudioRenderTarget::ConfigureVoicePool(int softVoiceLimit, int hardVoiceLimit) {
    mFModSystem.AudioRenderTarget::ConfigureVoicePool(softVoiceLimit, hardVoiceLimit);
}

// Reconstructed from eboot.elf at 0x276170.
FusionVoicePool* FmodRecordingAudioRenderTarget::GetVoicePool() {
    return mFModSystem.mVoicePool;
}

// Reconstructed from eboot.elf at 0x276180.
bool FmodRecordingAudioRenderTarget::TryBeginMix() {
    return false;
}

// Reconstructed from eboot.elf at 0x276190.
bool FmodRecordingAudioRenderTarget::EndMix() {
    return false;
}

// Reconstructed from eboot.elf at 0x2761A0.
void FmodRecordingAudioRenderTarget::Lock() {
    mFModSystem.Lock();
    AudioRenderTarget::Lock();
}

// Reconstructed from eboot.elf at 0x2761D0.
void FmodRecordingAudioRenderTarget::Unlock() {
    AudioRenderTarget::Unlock();
    mFModSystem.Unlock();
}

// Reconstructed from eboot.elf at 0x276200.
int FmodRecordingAudioRenderTarget::Update() {
    if (!mFModSystem.mUpdateStudio) {
        return FMOD_OK;
    }
    return mFModSystem.mStudioSystem->update();
}

// Reconstructed from eboot.elf at 0x276210.
void FmodRecordingAudioRenderTarget::AddPremixCallback(FmodPremixCallback* callback) {
    mFModSystem.AddPremixCallback(callback);
}

// Reconstructed from eboot.elf at 0x276230.
void FmodRecordingAudioRenderTarget::RemovePremixCallback(FmodPremixCallback* callback) {
    mFModSystem.RemovePremixCallback(callback);
}

// Reconstructed from eboot.elf at 0x276250.
void FmodRecordingAudioRenderTarget::ExecutePremixCallbacks(unsigned long mixCount) {
    Lock();
    mFModSystem.ExecutePremixCallbacks(mixCount);
    Unlock();
}

// Reconstructed from eboot.elf at 0x276290.
AudioMixer* FmodRecordingAudioRenderTarget::GetMixer() {
    return mFModSystem.GetMixer();
}

// Reconstructed from eboot.elf at 0x2762B0.
FModSystem* FmodRecordingAudioRenderTarget::GetOutputTarget() {
    return &mFModSystem;
}
