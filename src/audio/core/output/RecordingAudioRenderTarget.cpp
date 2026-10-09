#include "audio/core/output/RecordingAudioRenderTarget.h"

#include <new>
#include <unistd.h>

#include "audio/core/formats/WaveFile.h"
#include "audio/core/generators/AudioGenerator.h"
#include "entity/core/Entity.h"
#include "entity/core/TransEntityResource.h"
#include "os/memory/MemMgr.h"
#include "utl/streams/FileStream.h"

namespace {

// Bits per recorded sample. Name not in the reference map.
constexpr int kRecordingBitsPerSample = 16;
// FileStream mode the recording opens its file with. Name not in the
// reference map.
constexpr auto kRecordingFileMode = static_cast<FileMode>(4);
// Full scale of a 16-bit sample, at 0x136B90C. Name not in the reference
// map.
constexpr float kSampleScale = 32767.0F;

// The emitter component's layout as _CreateEmitter reaches it: the name of
// the render target it plays into, and its emitter interface. Both belong to
// the component's RuntimeData (constructed at 0x37710), which starts at
// +104. Names not in the reference map.
struct EmitterComponentView {
    // The Component base.
    unsigned char mComponentBase[104];
    // RuntimeData's members before the render target: the
    // CompositeGenerator at +128 in it and the emitter's lists and locks.
    unsigned char mRuntimeData[440];
    Symbol mRenderTarget;
    AudioEmitterCom mEmitter;
};

static_assert(offsetof(EmitterComponentView, mRenderTarget) == 544);
static_assert(offsetof(EmitterComponentView, mEmitter) == 552);

}  // namespace

// Reconstructed from eboot.elf at 0x11289D0. The mix buffer holds one
// interleaved buffer of the configured speakers.
RecordingAudioRenderTarget::RecordingAudioRenderTarget(
    Symbol name,
    const char* path,
    int bufferSize,
    int maxSoftwareChannels,
    int sampleRate,
    float gain,
    int speakerConfig)
    : AudioRenderTarget(name, sampleRate),
      mMixBuffer(nullptr),
      mWaveFile(nullptr),
      mStream(nullptr),
      mFramesWritten(0),
      mGeneratorHandle(0),
      mNumMixes(0),
      mStopRequested(true),
      mFinished(false),
      mGain(gain),
      mEntityResource(nullptr),
      mEmitter(nullptr) {
    mRecordThread.Init("Unknown Thread!");
    mBufferSize = bufferSize;
    mNumBuffers = 2;
    mMaxSoftwareChannels = maxSoftwareChannels;
    mSampleRate = sampleRate;
    mMixer.SetOutputSampleRate(sampleRate);
    mNumRawSpeakers = GetNumSpeakers(speakerConfig);
    mWaveFile = new WaveFile(sampleRate, kRecordingBitsPerSample, mNumRawSpeakers);
    mStream = new (MemAlloc(sizeof(FileStream), "FileStream", 0))
        FileStream(path, kRecordingFileMode, false);
    mMixBuffer = new float[static_cast<long>(mBufferSize) * mNumRawSpeakers];
    _CreateEmitter();
}

// Reconstructed from eboot.elf at 0x1128BB0.
void RecordingAudioRenderTarget::_ResetState() {
    mMixBuffer = nullptr;
    mWaveFile = nullptr;
    mStream = nullptr;
    mFramesWritten = 0;
    mGeneratorHandle = 0;
    mNumMixes = 0;
    mStopRequested = true;
    mFinished = false;
    mGain = 1.0F;
    mEntityResource = nullptr;
    mEmitter = nullptr;
}

// Reconstructed from eboot.elf at 0x1128BF0. The entity's resources load
// with the thread's entity flag cleared.
void RecordingAudioRenderTarget::_CreateEmitter() {
    mEntityResource = new TransEntityResource();
    GameObject* object = mEntityResource->CreateEntity()->CreateObject(0, 0);
    auto* component = reinterpret_cast<EmitterComponentView*>(
        object->CreateComponent(gAudioEmitterComClass, false));
    mEmitter = &component->mEmitter;
    mEmitter->Set2D(true);

    const bool savedFlag = gEntityThreadState.mEnterImmediately;
    gEntityThreadState.mEnterImmediately = false;
    component->mRenderTarget = GetOutputTarget()->mName;
    mEntityResource->LoadResources();
    mEntityResource->EnterEntity(mEntityResource->mEntity);
    gEntityThreadState.mEnterImmediately = savedFlag;
}

// Reconstructed from eboot.elf at 0x1128D30.
RecordingAudioRenderTarget::~RecordingAudioRenderTarget() {
    mStopRequested = true;
    mEmitter->GetCompositeGenerator()->KillLocked();
    delete mEntityResource;
    mEntityResource = nullptr;
    mEmitter = nullptr;
    delete mStream;
    delete mWaveFile;
    delete[] mMixBuffer;
    mRecordThread.mThread._ForceKillThread();
}

// Reconstructed from eboot.elf at 0x1128E50.
bool RecordingAudioRenderTarget::StreamFailed() {
    if (mWaveFile != nullptr && mStream != nullptr) {
        return mStream->Fail();
    }
    return true;
}

// Reconstructed from eboot.elf at 0x1128E70. The stream check's result is
// unused.
void RecordingAudioRenderTarget::StartRecording() {
    if (mWaveFile != nullptr && mStream != nullptr) {
        mStream->Fail();
    }
    mWaveFile->WriteFileHeader(*mStream);
    mStopRequested = false;
    StartAsyncRecording();
}

// Reconstructed from eboot.elf at 0x1128ED0.
void RecordingAudioRenderTarget::CheckStream() {
    if (mWaveFile != nullptr && mStream != nullptr) {
        mStream->Fail();
    }
}

// Reconstructed from eboot.elf at 0x1128F00.
void RecordingAudioRenderTarget::CheckStreamAgain() {
    if (mWaveFile != nullptr && mStream != nullptr) {
        mStream->Fail();
    }
}

// Reconstructed from eboot.elf at 0x1128F30.
void RecordingAudioRenderTarget::ReleaseRecording() {
    if (mStopRequested) {
        return;
    }
    mStopRequested = true;
    WaitForRecording();
    if (mWaveFile != nullptr && mStream != nullptr) {
        mStream->Fail();
    }
    delete mStream;
    mStream = nullptr;
    delete mWaveFile;
    mWaveFile = nullptr;
}

// Reconstructed from eboot.elf at 0x1129140.
void RecordingAudioRenderTarget::StopRecording() {
    if (mEmitter != nullptr) {
        mEmitter->GetCompositeGenerator()->KillLocked();
    }
}

// Reconstructed from eboot.elf at 0x1129160. Each pass polls the entity,
// waits while the watched generator is paused, mixes one buffer and writes
// it as 16-bit samples. The loop ends when stopped or when the watched
// generator is gone.
void RecordingAudioRenderTarget::RecordLoop() {
    while (!mStopRequested) {
        if (mEntityResource != nullptr) {
            mEntityResource->PollEntity(mEntityResource->mEntity);
        }
        if (mGeneratorHandle != 0) {
            AudioGenerator* generator = theSoundManager.LockIfOwned(mGeneratorHandle);
            if (generator == nullptr) {
                break;
            }
            --generator->mRefCount;
            generator = theSoundManager.LockIfOwned(mGeneratorHandle);
            if (generator != nullptr) {
                const AudioGenerator::State state = generator->GetState();
                --generator->mRefCount;
                if (state == AudioGenerator::kStatePaused) {
                    usleep(1000);
                    continue;
                }
            }
        }

        ExecutePremixCallbacks(mNumMixes++);
        Update();
        for (int frame = 0; frame < mBufferSize; ++frame) {
            for (int speaker = 0; speaker < mNumRawSpeakers; ++speaker) {
                const short sample = static_cast<short>(
                    kSampleScale * mMixBuffer[frame * mNumRawSpeakers + speaker] * mGain);
                mStream->Write(&sample, sizeof(sample));
            }
            ++mFramesWritten;
        }
    }
    mStream->Flush();
    mWaveFile->PatchDataSize(*mStream, mFramesWritten);
    mStream->Flush();
    mFinished = true;
}

// Reconstructed from eboot.elf at 0x1129360.
AudioRenderTarget* RecordingAudioRenderTarget::GetOutputTarget() {
    return this;
}
