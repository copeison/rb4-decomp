#include "audio/fmod/playback/FmodBufferedStreamGenerator.h"

#include <cmath>
#include <cstdlib>
#include <unistd.h>

#include "audio/fmod/io/FmodRecordingAudioRenderTarget.h"
#include "audio/fmod/playback/FmodAudioBusGenerator.h"
#include "audio/core/generators/GeneratorPool.h"
#include "audio/fmod/resources/FmodAudioStreamResource.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "utl/time/TimeMgr.h"

namespace {

// Requests in this format with streaming set use the buffered path. Name not
// in the reference map.
constexpr int kBufferedStreamFormat = 3;
constexpr FMOD_MODE kStreamMode2D = FMOD_NONBLOCKING | FMOD_ACCURATETIME | FMOD_CREATESTREAM;
constexpr FMOD_MODE kStreamMode3D = kStreamMode2D | FMOD_3D;
// Converts 16-bit samples to [-1, 1). Names not in the reference map.
constexpr float kShortToFloat = 1.0F / 32768.0F;
constexpr float kMsPerSecond = 1000.0F;
constexpr float kSecondsPerMs = 0.001F;
// Unset loop points. Name not in the reference map.
constexpr float kNoLoopMs = -1.0F;
// The sync rate is clamped to this many times the stream rate. Name not in
// the reference map.
constexpr float kMaxSyncRateScale = 20.0F;
// FMOD_ERR_FILE_EOF also counts as a successful read.
constexpr int kReadEndOfFileBit = 0x10;

FMOD::System* LowLevelSystemFor(AudioRenderTarget* target) {
    FModSystem* system = FModSystemForTarget(target);
    return system != nullptr && system->mStudioSystem != nullptr ? system->mLowLevelSystem : nullptr;
}

StreamReaderThread& ReaderThreadOf(FmodBufferedStreamGenerator& generator) {
    return static_cast<FmodBufferedStreamGeneratorManager*>(generator.mManager)->mReaderThread;
}

// Takes every queued reader off the reader thread and waits until none is
// reading. Inlined into SeekToMs and Release. Name not in the reference map.
void CancelReaders(FmodBufferedStreamGenerator& generator) {
    while (!generator.mReaders.empty()) {
        bool idle = true;
        for (StreamReader& reader : generator.mReaders) {
            if (reader.mState != StreamReader::kStateIdle && reader.mState != StreamReader::kStateDone) {
                ReaderThreadOf(generator).RemoveReader(&reader);
                idle = false;
            }
        }
        if (idle) {
            break;
        }
        usleep(0);
    }
}

// Whether a reader may be repositioned. Name not in the reference map.
bool ReaderIsFree(const StreamReader& reader) {
    return reader.mState == StreamReader::kStateDone || reader.mState == StreamReader::kStateIdle;
}

// The index before `index` in a ring of `count`. Name not in the reference
// map.
int PreviousIndex(int index, int count) {
    int previous = (index - 1) % count;
    return previous < 0 ? previous + count : previous;
}

}  // namespace

Symbol FmodBufferedStreamGenerator::sTypeId("");
float FmodBufferedStreamGenerator::sSyncLeadMs = 30.0F;

// Inlined into the pool setup at 0x26E000. The reader ring, the decode
// buffer and the sync smoother start empty; the other fields are set by
// Init and Setup.
FmodBufferedStreamGenerator::FmodBufferedStreamGenerator() {}

// Reconstructed from eboot.elf at 0x26ADA0. Without a bus generator the
// readers and the stream are released again.
bool FmodBufferedStreamGenerator::Setup(ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args) {
    if (!_OpenSound(resource)) {
        return false;
    }
    mLoopEndMs = kNoLoopMs;
    mLoopStartMs = kNoLoopMs;
    mLoopStartFrame = -1;
    mLoopEndFrame = -1;
    mSpeed = 1.0F;
    mLoopDirty = false;
    mSyncEnabled = false;
    _InitBuffers(static_cast<const BufferedStreamPlayArgs&>(args));
    if (!_AcquireBusGenerator(args)) {
        mReaders.clear();
        mDecodeBuffer.Release();
        mSound->release();
        return false;
    }
    mState = args.mStartPaused ? kStatePaused : kStatePlaying;
    return true;
}

// Reconstructed from eboot.elf at 0x26AFF0. The bus side renders stereo in
// 128-sample blocks at the engine sample rate.
void FmodBufferedStreamGenerator::Init(AudioGeneratorManager* manager, int index) {
    _InitTypeId();
    mManager = manager;
    mIndex = index;
    mBusGenerator = nullptr;
    AudioBus::Prepare(static_cast<float>(Audio::GetSamplesPerSecond()), 2, 128, false);
}

// Reconstructed from eboot.elf at 0x26B060. A sound off the default 2D
// emitter is opened as a 3D stream. The resource is kept once FMOD accepts
// the request.
bool FmodBufferedStreamGenerator::_OpenSound(const ResourcePtr<FmodAudioStreamResource>& resource) {
    FMOD::System* lowLevel = LowLevelSystemFor(mRenderTarget);
    const FMOD_MODE mode =
        mEmitter != nullptr && mEmitter != theSoundManager.GetDefault2DEmitter() ? kStreamMode3D : kStreamMode2D;
    FMOD_RESULT result = lowLevel->createSound(resource->mFile.Str(), mode, nullptr, &mSound);
    mOpening = true;
    if (result != FMOD_OK) {
        return false;
    }
    mResource = resource.Get();
    return true;
}

// Reconstructed from eboot.elf at 0x26B140.
void FmodBufferedStreamGenerator::_DisableSyncUnlocked() {
    mSyncEnabled = false;
}

// Reconstructed from eboot.elf at 0x26B150. Each reader decodes stereo into
// its block of the decode buffer; reader i starts at frame i times the block
// size.
void FmodBufferedStreamGenerator::_InitBuffers(const BufferedStreamPlayArgs& args) {
    mWaitingForStart = true;
    mTimelineMs = 0.0F;
    mCurrentReader = 0;
    mReadPos = 0.0F;
    mFrequencyRatio = 1.0F;
    mBuffersBehind = args.mBuffersBehind;
    mFramesPerBuffer = args.mFramesPerBuffer;
    mClearedOnSetup = 0;
    mStartReader = args.mBuffersBehind;
    mDecodeBuffer.Configure(
        AudioBufferConfig(2, args.mNumBuffers * mFramesPerBuffer, static_cast<float>(mSampleRate), true),
        AudioBufferBase::kCleanupFree);
    mReaders.resize(args.mNumBuffers);
    int stride = mDecodeBuffer.mConfig.mInterleaved ? mDecodeBuffer.mConfig.mNumChannels : 1;
    short* storage = mDecodeBuffer.mChannelData[0];
    int startFrame = -mFramesPerBuffer;
    for (int index = 0; index < args.mNumBuffers; ++index) {
        StreamReader& reader = mReaders[index];
        reader.Setup(
            mSound,
            2,
            storage + static_cast<long>(index * stride) * mFramesPerBuffer,
            2 * mDecodeBuffer.mNumFrames * mDecodeBuffer.mNumChannels,
            [index, this]() {
                static_cast<void>(index);
                static_cast<void>(this);
            });
        startFrame += mFramesPerBuffer;
        _SetReaderPosition(reader, startFrame, false);
    }
}

// Reconstructed from eboot.elf at 0x26B490. The bus generator renders this
// generator's AudioBus at its render target.
bool FmodBufferedStreamGenerator::_AcquireBusGenerator(const PlayArgs& args) {
    auto* manager = theSoundManager._GetManager(FmodAudioBusGeneratorManager::Id());
    if (manager == nullptr) {
        return false;
    }
    mBusGenerator = GeneratorPool::Allocate<FmodAudioBusGenerator>(*manager, mRenderTarget, args.mEmitter);
    if (mBusGenerator == nullptr) {
        return false;
    }
    mBusGenerator->Setup(this, args, nullptr);
    return true;
}

// Reconstructed from eboot.elf at 0x26B650.
void FmodBufferedStreamGenerator::_SetReaderPosition(StreamReader& reader, int startFrame, bool loop) {
    reader.mStartFrame = startFrame;
    reader.mSegments[0].mStart = startFrame;
    int count = mFramesPerBuffer;
    int segment = 0;
    if (loop) {
        int firstCount = mLoopEndFrame - startFrame;
        reader.mSegments[0].mCount = firstCount;
        count -= firstCount;
        reader.mSegments[1].mStart = mLoopStartFrame;
        segment = 1;
    }
    reader.mSegments[segment].mCount = count;
    reader.mSegments[segment + 1].mCount = 0;
}

// Reconstructed from eboot.elf at 0x26B6A0 (0x26B930 deletes).
FmodBufferedStreamGenerator::~FmodBufferedStreamGenerator() {
    KillLocked();
}

// Reconstructed from eboot.elf at 0x26B9A0. The voice ends with its bus
// generator.
bool FmodBufferedStreamGenerator::Poll() {
    if (mState == kStateStopped) {
        return false;
    }
    if (!mBusGenerator->Poll()) {
        mState = kStateStopped;
        return false;
    }
    if (mState == kStateStopping) {
        if (mBusGenerator->GetState() == kStateStopped) {
            mState = kStateStopped;
            return false;
        }
        return true;
    }
    if (mOpening) {
        _FinishOpen();
    }
    _UpdateSync();
    return true;
}

// Reconstructed from eboot.elf at 0x26BAD0. A mono stream is widened by its
// readers.
void FmodBufferedStreamGenerator::_FinishOpen() {
    FMOD_OPENSTATE openState;
    unsigned int percentBuffered = 0;
    bool starving = false;
    bool diskBusy = false;
    mSound->getOpenState(&openState, &percentBuffered, &starving, &diskBusy);
    if (openState != FMOD_OPENSTATE_READY) {
        return;
    }
    LockBus();
    FMOD_SOUND_TYPE type;
    FMOD_SOUND_FORMAT format;
    int channels;
    int bits;
    mSound->getFormat(&type, &format, &channels, &bits);
    int priority;
    mSound->getDefaults(&mSoundFrequency, &priority);
    mSound->getLength(&mLengthFrames, FMOD_TIMEUNIT_PCM);
    if (channels == 1) {
        for (int index = 0; index < static_cast<int>(mReaders.size()); ++index) {
            mReaders[index].mNumChannels = 1;
        }
    }
    mFrequencyRatio = mSoundFrequency / static_cast<float>(mSampleRate);
    mSound->seekData(0);
    for (StreamReader& reader : mReaders) {
        ReaderThreadOf(*this).AddReader(&reader);
    }
    mOpening = false;
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26BC70. The rate is the target's change
// per second of the clock, clamped to 20 times the stream rate either way.
void FmodBufferedStreamGenerator::_UpdateSync() {
    if (!mSyncEnabled) {
        return;
    }
    LockBus();
    float now = TheTimeMgr->mClock.Seconds();
    float elapsed = now - mSyncLastTime;
    mSyncLastTime = now;
    int moved = mSyncTargetFrame - mSyncPrevTargetFrame;
    mSyncPrevTargetFrame = mSyncTargetFrame;
    float minimum = mSoundFrequency * -kMaxSyncRateScale;
    float maximum = mSoundFrequency * kMaxSyncRateScale;
    float rate = static_cast<float>(moved) / elapsed;
    float clamped = minimum > rate ? minimum : rate;
    mSyncRate = maximum < rate ? maximum : clamped;
    mSyncSmoother.Smooth(mSyncRate, elapsed * mSyncTimeScale);
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26BD40.
void FmodBufferedStreamGenerator::Pause() {
    mBusGenerator->Pause();
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x26BD70.
void FmodBufferedStreamGenerator::Continue() {
    mBusGenerator->Continue();
    mState = kStatePlaying;
}

// Reconstructed from eboot.elf at 0x26BDA0.
float FmodBufferedStreamGenerator::GetElapsedMs() {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x26BDB0.
float FmodBufferedStreamGenerator::GetTimelineMs() {
    return mTimelineMs;
}

// Reconstructed from eboot.elf at 0x26BDC0. The ring is rebuilt around the
// target frame with mBuffersBehind blocks behind it; near the start it
// begins at frame zero instead. Playback waits for the block at the target.
void FmodBufferedStreamGenerator::SeekToMs(float ms) {
    LockBus();
    if (!mOpening) {
        mTimelineMs = ms;
        int frame = static_cast<int>(ms * kSecondsPerMs * mSoundFrequency);
        CancelReaders(*this);
        mWaitingForStart = true;
        int numReaders = static_cast<int>(mReaders.size());
        mCurrentReader = numReaders / 2;
        mReadPos = 0.0F;
        int startFrame = frame - mFramesPerBuffer * mBuffersBehind;
        if (startFrame < 0) {
            startFrame = 0;
            mCurrentReader = frame / mFramesPerBuffer;
            mReadPos = static_cast<float>(frame - mCurrentReader * mFramesPerBuffer);
        }
        int first = mCurrentReader - mBuffersBehind;
        if (first < 0) {
            first = 0;
        }
        int readerStart = startFrame - mFramesPerBuffer;
        for (int index = first; index < static_cast<int>(mReaders.size()); ++index) {
            readerStart += mFramesPerBuffer;
            _SetReaderPosition(mReaders[index], readerStart, false);
            ReaderThreadOf(*this).AddReader(&mReaders[index]);
        }
        mStartReader = mCurrentReader + mBuffersBehind;
    }
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26C040. The render thread reads the
// speed under the bus lock.
void FmodBufferedStreamGenerator::SetSpeed(float speed, bool) {
    LockBus();
    mSpeed = speed;
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26C080.
float FmodBufferedStreamGenerator::GetSpeed(bool* changing) {
    if (changing != nullptr) {
        *changing = false;
    }
    return mSpeed;
}

// Reconstructed from eboot.elf at 0x26C0A0.
void FmodBufferedStreamGenerator::ClearLoopPoints() {
    LockBus();
    mLoopEndMs = kNoLoopMs;
    mLoopStartMs = kNoLoopMs;
    mLoopDirty = true;
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26C0F0.
void FmodBufferedStreamGenerator::SetLoopPoints(float endMs, float startMs) {
    LockBus();
    mLoopEndMs = endMs;
    mLoopStartMs = startMs;
    mLoopDirty = true;
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26C150.
void FmodBufferedStreamGenerator::Stop() {
    mBusGenerator->Stop();
    mState = kStateStopping;
}

// Reconstructed from eboot.elf at 0x26C180.
void FmodBufferedStreamGenerator::Kill() {
    mBusGenerator->KillLocked();
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x26C1B0. Releases the bus generator, the
// readers and the stream, then returns the voice to the pool.
void FmodBufferedStreamGenerator::Release() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->Release();
        mBusGenerator = nullptr;
    }
    CancelReaders(*this);
    mReaders.clear();
    mDecodeBuffer.Release();
    mSound->release();
    GeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x26C480. Up to half the ring is kept
// ahead of the current block and the rest behind it, each block following
// its neighbour; a reader still decoding stops the walk.
void FmodBufferedStreamGenerator::_RefillBuffers() {
    if (mWaitingForStart) {
        return;
    }
    int numReaders = static_cast<int>(mReaders.size());
    int aheadEnd = (mCurrentReader + numReaders / 2) % numReaders;
    int framesPerBuffer = mFramesPerBuffer;
    int index = (mCurrentReader + 1) % numReaders;
    int currentStart = mReaders[mCurrentReader].mStartFrame;
    if (index != aheadEnd) {
        int expected = currentStart + framesPerBuffer;
        bool placed = true;
        while (expected == mReaders[index].mStartFrame) {
            expected += framesPerBuffer;
            index = (index + 1) % numReaders;
            if (index == aheadEnd) {
                placed = false;
                break;
            }
        }
        if (placed) {
            while (index != aheadEnd && ReaderIsFree(mReaders[index])) {
                _SetReaderPosition(mReaders[index], expected, false);
                ReaderThreadOf(*this).AddReader(&mReaders[index]);
                framesPerBuffer = mFramesPerBuffer;
                index = (index + 1) % static_cast<int>(mReaders.size());
                expected += framesPerBuffer;
            }
            currentStart = mReaders[mCurrentReader].mStartFrame;
        }
    }

    int expected = currentStart - framesPerBuffer;
    numReaders = static_cast<int>(mReaders.size());
    if (expected <= -framesPerBuffer) {
        return;
    }
    int behindEnd = PreviousIndex(aheadEnd, numReaders);
    index = PreviousIndex(mCurrentReader, numReaders);
    if (index == behindEnd) {
        return;
    }
    while (expected == mReaders[index].mStartFrame) {
        expected -= framesPerBuffer;
        index = PreviousIndex(index, numReaders);
        if (index == behindEnd) {
            return;
        }
    }
    while (index != behindEnd && expected > -framesPerBuffer && ReaderIsFree(mReaders[index])) {
        _SetReaderPosition(mReaders[index], expected, false);
        ReaderThreadOf(*this).AddReader(&mReaders[index]);
        numReaders = static_cast<int>(mReaders.size());
        index = PreviousIndex(index, numReaders);
        framesPerBuffer = mFramesPerBuffer;
        expected -= framesPerBuffer;
    }
}

// Reconstructed from eboot.elf at 0x26C720. Resamples the decoded stereo
// into the block: with the optimal six-point interpolator while synced, and
// linearly at the speed otherwise. Reaching the end of the stream or a failed
// read stops the voice; an unread block renders silence until it arrives.
// The binary returns whatever UnlockBus leaves; the caller ignores it.
bool FmodBufferedStreamGenerator::Process(AudioBuffer<float>& buffer) {
    LockBus();
    if (mWaitingForStart) {
        if (mReaders[mStartReader].mState != StreamReader::kStateDone) {
            buffer.Clear();
            UnlockBus();
            return true;
        }
        mWaitingForStart = false;
    }
    StreamReader* current = &mReaders[mCurrentReader];
    if (_CurrentReaderFailed()) {
        buffer.Clear();
        if (mState != kStateStopped && mState != kStateStopping) {
            mState = kStateStopping;
            mBusGenerator->Stop();
        }
        UnlockBus();
        return true;
    }
    if (mLoopDirty) {
        mLoopDirty = false;
        if (mLoopEndMs == kNoLoopMs && mLoopStartMs == kNoLoopMs) {
            mLoopStartFrame = -1;
            mLoopEndFrame = -1;
        } else {
            mLoopEndFrame = mLoopEndMs != kNoLoopMs ? static_cast<int>(mLoopEndMs * kSecondsPerMs * mSoundFrequency)
                                                     : static_cast<int>(mLengthFrames);
            mLoopStartFrame = static_cast<int>(mLoopStartMs * kSecondsPerMs * mSoundFrequency);
        }
    }

    float* left = buffer.mChannelData[0];
    float* right = buffer.mChannelData[1];
    const bool syncing = mSyncEnabled;
    if (_GetPlaybackSpeed() > 0.0F && static_cast<float>(current->mFramesRead) <= mReadPos) {
        buffer.Clear();
        if (mState != kStateStopped && mState != kStateStopping) {
            mState = kStateStopping;
            mBusGenerator->Stop();
        }
        UnlockBus();
        return true;
    }
    if (!(_GetPlaybackSpeed() > 0.0F) && current->mStartFrame <= 0 &&
        !(mReadPos > static_cast<float>(-current->mStartFrame))) {
        buffer.Clear();
        UnlockBus();
        return true;
    }

    const int numFrames = buffer.mConfig.mNumFrames;
    int frame = 0;
    bool endOfStream = false;
    bool movedReader = false;
    if (numFrames > 0) {
        const int startFrame = current->mStartFrame;
        const bool atStreamEnd = startFrame + current->mFramesRead >= static_cast<int>(mLengthFrames);
        short* const ringBegin = mDecodeBuffer.mChannelData[0];
        short* const ringEnd = ringBegin + 2 * mDecodeBuffer.mConfig.mNumFrames;
        short* data = current->mBuffer;
        int whole = static_cast<int>(std::floor(mReadPos));
        short* sample = data + 2 * whole;
        while (true) {
            const float fraction = mReadPos - static_cast<float>(whole);
            if (syncing) {
                if (_GetSyncSpeed() != 0.0F) {
                    // Six frames from two before the position, wrapping in the
                    // ring; past the stream end the last frame repeats.
                    float leftIn[6];
                    float rightIn[6];
                    short* cursor = sample - 4;
                    if (cursor < ringBegin) {
                        cursor = ringEnd - (ringBegin - cursor);
                    }
                    for (int tap = 0; tap < 6; ++tap) {
                        if (cursor >= ringEnd) {
                            cursor = atStreamEnd ? ringEnd - 2 : ringBegin + (cursor - ringEnd);
                        }
                        leftIn[tap] = cursor[0];
                        rightIn[tap] = cursor[1];
                        cursor += 2;
                        if (cursor < ringBegin) {
                            cursor = ringEnd - (ringBegin - cursor);
                        }
                    }
                    left[frame] = _InterpolateOptimal(leftIn, fraction) * kShortToFloat;
                    right[frame] = _InterpolateOptimal(rightIn, fraction) * kShortToFloat;
                    float timelineMs =
                        (static_cast<float>(mReaders[mCurrentReader].mStartFrame) + mReadPos) / mSoundFrequency *
                        kMsPerSecond;
                    mTimelineMs = timelineMs > 0.0F ? timelineMs : 0.0F;
                    mReadPos += _GetSyncSpeed();
                } else {
                    left[frame] = 0.0F;
                    right[frame] = 0.0F;
                }
            } else {
                short* next = sample + 2;
                short* here = sample;
                if (here < ringBegin) {
                    here = ringEnd - (ringBegin - here);
                }
                short* nextLeft;
                short* nextRight;
                if (next < ringEnd) {
                    nextLeft = next;
                    nextRight = next + 1;
                } else if (atStreamEnd) {
                    nextLeft = here;
                    nextRight = here + 1;
                } else {
                    nextLeft = ringBegin + (next - ringEnd);
                    nextRight = nextLeft + 1;
                }
                left[frame] =
                    (static_cast<float>(here[0]) + static_cast<float>(*nextLeft - here[0]) * fraction) * kShortToFloat;
                right[frame] =
                    (static_cast<float>(here[1]) + static_cast<float>(*nextRight - here[1]) * fraction) * kShortToFloat;
                mReadPos += mSpeed * mFrequencyRatio;
            }
            if (startFrame <= 0 && 0.0F > mReadPos) {
                mReadPos = 0.0F;
                left[frame] = 0.0F;
                right[frame] = 0.0F;
            }
            if (atStreamEnd) {
                float framesRead = static_cast<float>(mReaders[mCurrentReader].mFramesRead);
                if (!(framesRead >= mReadPos)) {
                    mReadPos = framesRead;
                }
            }
            if (startFrame <= 0 && (!(mSpeed > 0.0F) || mSyncEnabled) &&
                static_cast<float>(-mReaders[mCurrentReader].mStartFrame) >= mReadPos) {
                break;
            }

            float position = mReadPos;
            bool moved = false;
            if (-1.0F >= position) {
                mCurrentReader = PreviousIndex(mCurrentReader, static_cast<int>(mReaders.size()));
                position += static_cast<float>(mFramesPerBuffer);
                moved = true;
            } else if (!atStreamEnd && position >= static_cast<float>(mFramesPerBuffer)) {
                position -= static_cast<float>(mFramesPerBuffer);
                mCurrentReader = (mCurrentReader + 1) % static_cast<int>(mReaders.size());
                moved = true;
            } else if (position >= static_cast<float>(mReaders[mCurrentReader].mFramesRead)) {
                endOfStream = true;
                break;
            }
            if (moved) {
                mReadPos = position;
                data = mReaders[mCurrentReader].mBuffer;
                movedReader = true;
            }
            if (movedReader) {
                const StreamReader& reader = mReaders[mCurrentReader];
                if (reader.mState == StreamReader::kStateQueued || reader.mState == StreamReader::kStateReading) {
                    break;
                }
                if (_CurrentReaderFailed()) {
                    buffer.Clear();
                    if (mState != kStateStopped && mState != kStateStopping) {
                        mState = kStateStopping;
                        mBusGenerator->Stop();
                    }
                    UnlockBus();
                    return true;
                }
            }
            whole = static_cast<int>(std::floor(position));
            sample = data + 2 * whole;
            if (++frame >= numFrames) {
                break;
            }
        }
    }
    for (; frame < numFrames; ++frame) {
        left[frame] = 0.0F;
        right[frame] = 0.0F;
    }
    if (endOfStream) {
        mState = kStateStopping;
        mBusGenerator->Stop();
    }
    float timelineMs =
        (static_cast<float>(mReaders[mCurrentReader].mStartFrame) + mReadPos) / mSoundFrequency * kMsPerSecond;
    mTimelineMs = timelineMs > 0.0F ? timelineMs : 0.0F;
    if (movedReader) {
        _RefillBuffers();
    }
    buffer.mCleared = false;
    UnlockBus();
    return true;
}

// Reconstructed from eboot.elf at 0x26D880.
bool FmodBufferedStreamGenerator::_CurrentReaderFailed() const {
    return (mReaders[mCurrentReader].mResult | kReadEndOfFileBit) != kReadEndOfFileBit;
}

// Reconstructed from eboot.elf at 0x26D8D0. Playback stops at the target
// rather than overshooting it in the direction the target moves.
float FmodBufferedStreamGenerator::_GetSyncSpeed() const {
    int now = static_cast<int>(mTimelineMs * kSecondsPerMs * mSoundFrequency);
    if (std::abs(mSyncTargetFrame - now) <= 0) {
        return 0.0F;
    }
    float rate = mSyncSmoother.mValue;
    if (rate < 0.0F && mSyncTargetFrame > now) {
        return 0.0F;
    }
    if (rate > 0.0F && mSyncTargetFrame < now) {
        return 0.0F;
    }
    return rate / static_cast<float>(mSampleRate);
}

// Reconstructed from eboot.elf at 0x26D940. The vectorized original computes
// the same polynomial from float-rounded coefficients.
float FmodBufferedStreamGenerator::_InterpolateOptimal(
    const float* samples, float fraction) const {
    const float z = fraction - 0.5F;
    const float even1 = samples[3] + samples[2];
    const float odd1 = samples[3] - samples[2];
    const float even2 = samples[4] + samples[1];
    const float odd2 = samples[4] - samples[1];
    const float even3 = samples[5] + samples[0];
    const float odd3 = samples[5] - samples[0];

    const float c0 = even1 * 0.426859825850F + even2 * 0.0723812356591F +
        even3 * 0.000758930807933F;
    const float c1 = odd1 * 0.358317732811F + odd2 * 0.204516440630F +
        odd3 * 0.00562658812851F;
    const float c2 = even1 * -0.217009171844F + even2 * 0.200513765216F +
        even3 * 0.0164954103529F;
    const float c3 = odd1 * -0.251127153635F + odd2 * 0.0422302596271F +
        odd3 * 0.0248872749507F;
    const float c4 = even1 * 0.0416694656014F + even2 * -0.0625042021275F +
        even3 * 0.0208347346634F;
    const float c5 = odd1 * 0.0834979936481F + odd2 * -0.0417491272092F +
        odd3 * 0.00834987871349F;
    return ((((c5 * z + c4) * z + c3) * z + c2) * z + c1) * z + c0;
}

// Reconstructed from eboot.elf at 0x26DAB0. The smoother restarts at the
// stream rate, so playback first runs at unit speed.
float FmodBufferedStreamGenerator::EnableSync(float levelRate, float timeScale) {
    LockBus();
    mSyncEnabled = true;
    mSyncPrevTargetFrame = static_cast<int>(mTimelineMs * kSecondsPerMs * mSoundFrequency);
    float targetMs = mTimelineMs + sSyncLeadMs;
    mSyncTargetFrame = static_cast<int>(targetMs * (kSecondsPerMs * mSoundFrequency));
    mSyncSmoother.ForceValue(mSoundFrequency, false);
    mSyncRate = 0.0F;
    mSyncLastTime = TheTimeMgr->mClock.Seconds();
    mSyncSmoother.mLevelRate = levelRate;
    mSyncSmoother.mTrendRate = 0.0F;
    mSyncTimeScale = timeScale;
    UnlockBus();
    return targetMs;
}

// Reconstructed from eboot.elf at 0x26DBA0. Negative times clamp to zero.
void FmodBufferedStreamGenerator::SetSyncTargetMs(float ms) {
    float clamped = 0.0F > ms ? 0.0F : ms;
    mSyncTargetFrame = static_cast<int>(clamped * kSecondsPerMs * mSoundFrequency);
}

// Reconstructed from eboot.elf at 0x26DBD0.
void FmodBufferedStreamGenerator::DisableSync() {
    LockBus();
    mSyncEnabled = false;
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26E320.
void FmodBufferedStreamGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    if (mBusGenerator != nullptr) {
        mBusGenerator->SetGain(gain, fadeSecs, option);
    }
}

// Reconstructed from eboot.elf at 0x26E340.
float FmodBufferedStreamGenerator::GetGain() const {
    return mBusGenerator != nullptr ? mBusGenerator->GetGain() : 0.0F;
}

// Reconstructed from eboot.elf at 0x26E360.
void FmodBufferedStreamGenerator::SetMute(bool mute, bool immediate) {
    if (mBusGenerator != nullptr) {
        mBusGenerator->SetMute(mute, immediate);
    }
}

// Reconstructed from eboot.elf at 0x26E380.
bool FmodBufferedStreamGenerator::GetMute() const {
    return mBusGenerator != nullptr && mBusGenerator->GetMute();
}

// Reconstructed from eboot.elf at 0x26E3A0.
void FmodBufferedStreamGenerator::_InitTypeId() {
    sTypeId = Symbol("FmodBufferedStreamGenerator");
}

// Reconstructed from eboot.elf at 0x26E3F0.
AudioGenerator* FmodBufferedStreamGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x26E410.
Symbol FmodBufferedStreamGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x26AB60. Only streaming requests in the
// buffered format are accepted.
AudioGenerator* FmodBufferedStreamGeneratorManager::Play(const PlayArgs& args) {
    ResourcePtr<FmodAudioStreamResource> resource = FmodAudioStreamResource::Find(args.mName);
    if (!resource || resource->Fail()) {
        return nullptr;
    }
    if (args.mFormat != kBufferedStreamFormat || !args.mStreaming) {
        return nullptr;
    }
    return _AllocateAndSetUpGenerator(resource, args);
}

// Reconstructed from eboot.elf at 0x26AC30.
FmodBufferedStreamGenerator* FmodBufferedStreamGeneratorManager::_AllocateAndSetUpGenerator(
    ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args) {
    AudioEmitter* emitter =
        args.mEmitter != nullptr ? args.mEmitter : theSoundManager.GetDefault2DEmitter();
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    auto* generator =
        GeneratorPool::Allocate<FmodBufferedStreamGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    return generator->Setup(resource, args) ? generator : nullptr;
}

// Reconstructed from eboot.elf at 0x26AAC0.
void FmodBufferedStreamGeneratorManager::Init() {
    AudioGeneratorManager::Init();
    mReaderThread.StartAsyncPoll();
}

// Reconstructed from eboot.elf at 0x26DC00.
int FmodBufferedStreamGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x26DC10.
Symbol FmodBufferedStreamGeneratorManager::GetId() {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("FmodBufferedStreamGeneratorManager");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x26DCB0.
Symbol FmodBufferedStreamGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".mp3");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0x26DD50.
AudioGenerator* FmodBufferedStreamGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x26DDC0.
void FmodBufferedStreamGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26DE30.
void FmodBufferedStreamGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26DEA0.
void FmodBufferedStreamGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x26DFF0.
void FmodBufferedStreamGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x26E000.
void FmodBufferedStreamGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26E270.
bool FmodBufferedStreamGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26AAE0.
FmodBufferedStreamGeneratorManager::~FmodBufferedStreamGeneratorManager() {
    mReaderThread.QuitAsyncPoll();
}
