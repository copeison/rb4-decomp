#include "audio/fmod/playback/FmodAudioStreamGenerator.h"

#include <cmath>

#include "audio/core/components/AudioEmitterCom.h"
#include "audio/fmod/io/FmodRecordingAudioRenderTarget.h"
#include "audio/core/generators/GeneratorPool.h"
#include "audio/fmod/resources/FmodAudioStreamResource.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "math/transform/Transform.h"
#include "utl/time/Timer.h"

namespace {

constexpr int kBusRetries = 10;
constexpr unsigned int kNoPendingSeek = 0xFFFFFFFF;
// The buffered-stream format handled by FmodBufferedStreamGeneratorManager.
// Name not in the reference map.
constexpr int kBufferedStreamFormat = 3;
constexpr FMOD_MODE kStreamMode2D = FMOD_NONBLOCKING | FMOD_ACCURATETIME | FMOD_CREATESTREAM;
constexpr FMOD_MODE kStreamMode3D = kStreamMode2D | FMOD_3D;

float DecibelsToGain(float db) {
    return std::pow(10.0F, db * 0.05F);
}

FMOD::System* LowLevelSystemFor(AudioRenderTarget* target) {
    FModSystem* system = FModSystemForTarget(target);
    return system != nullptr && system->mStudioSystem != nullptr ? system->mLowLevelSystem : nullptr;
}

}  // namespace

Symbol FmodAudioStreamGenerator::sTypeId("");

FmodAudioStreamGenerator::FmodAudioStreamGenerator() {
    mPlayTimer.mStartCycles = 0;
    mPlayTimer.mElapsedCycles = 0;
    mPlayTimer.mRunning = 0;
}

// Reconstructed from eboot.elf at 0x2692B0.
void FmodAudioStreamGenerator::Init(AudioGeneratorManager* manager, int index) {
    _InitTypeId();
    mManager = manager;
    mIndex = index;
}

// Reconstructed from eboot.elf at 0x2693B0.
FmodAudioStreamGenerator::~FmodAudioStreamGenerator() {
    KillLocked();
}

// Reconstructed from eboot.elf at 0x268EF0. A sound off the default 2D
// emitter is opened as a 3D stream.
bool FmodAudioStreamGenerator::Setup(
    ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args) {
    FMOD::System* lowLevel = LowLevelSystemFor(mRenderTarget);
    mState = kStateInit;
    mSound = nullptr;
    mChannel = nullptr;
    mStudioBus = nullptr;
    mBusRetries = kBusRetries;
    mLengthMs = 0;
    mPositionMs = 0;
    mPendingSeekMs = 0;
    mPlayTimer.Reset();
    mLoopEndMs = -1.0F;
    mLoopStartMs = -1.0F;
    mLoopDirty = false;
    mSpeed = 1.0F;
    mSpeedDirty = false;
    mFrequency = -1.0F;
    mLastPollMs = 0.0F;

    if (args.mHasInitialGain) {
        SetGain(
            DecibelsToGain(args.mInitialGainDb),
            args.mInitialGainFadeSecs,
            static_cast<PostFadeOption>(args.mInitialGainPostFade));
    } else {
        mGain.Begin(1.0F);
        mGain.Snap();
        mStopAfterGain = false;
    }
    mMuteGain.Begin(args.mStartMuted ? 0.0F : 1.0F);
    mMuted = args.mStartMuted;
    mMuteGain.Snap();

    const FMOD_MODE mode =
        mEmitter != nullptr && mEmitter != theSoundManager.GetDefault2DEmitter()
        ? kStreamMode3D
        : kStreamMode2D;
    if (lowLevel->createSound(resource->mFile.Str(), mode, nullptr, &mSound) != FMOD_OK) {
        return false;
    }
    mPauseRequested = args.mStartPaused;
    mState = kStateReady;
    if (args.mRoute == PlayArgs::kRouteBus) {
        auto* system = static_cast<FModSystem*>(mRenderTarget);
        if (system->mStudioSystem->getBus(args.mRoutePath.Str(), &mStudioBus) != FMOD_OK) {
            mStudioBus = nullptr;
        } else {
            _CheckBusLoaded(mStudioBus);
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2692E0.
void FmodAudioStreamGenerator::_CheckBusLoaded(FMOD::Studio::Bus* bus) {
    gFmodBusInterface->IsBusLoaded(_GetBusPath(bus));
}

// Reconstructed from eboot.elf at 0x26A0F0.
void FmodAudioStreamGenerator::_CheckBusKnown(FMOD::Studio::Bus* bus) {
    gFmodBusInterface->IsBusKnown(_GetBusPath(bus));
}

// Reconstructed from eboot.elf at 0x26A1C0.
Symbol FmodAudioStreamGenerator::_GetBusPath(FMOD::Studio::Bus* bus) {
    char path[256] = {};
    bus->getPath(path, sizeof(path), nullptr);
    return Symbol(path);
}

// Reconstructed from eboot.elf at 0x2694D0. Starts the channel once ready,
// applies speed and loop changes, advances both gain ramps against the
// channel position, and mirrors the requested pause state.
bool FmodAudioStreamGenerator::Poll() {
    if (mState == kStateStopped) {
        return false;
    }
    if (mChannel != nullptr) {
        bool playing = false;
        const FMOD_RESULT result = mChannel->isPlaying(&playing);
        if (result == FMOD_ERR_INVALID_HANDLE) {
            mChannel = nullptr;
            mState = kStateStopping;
        } else if (result != FMOD_OK) {
            mState = kStateStopping;
        }
    }
    if (mState == kStateReady) {
        _TryStartChannel();
    } else if (mState == kStateStopping) {
        if (mChannel != nullptr) {
            mChannel->stop();
            mChannel = nullptr;
        }
        mState = kStateStopped;
        return false;
    }
    if (mChannel == nullptr) {
        return true;
    }

    if (mSpeedDirty && mFrequency > 0.0F) {
        mChannel->setFrequency(mFrequency * mSpeed);
        mSpeedDirty = false;
    }
    if (mLoopDirty) {
        if (mLoopEndMs >= 0.0F) {
            mChannel->setLoopPoints(
                static_cast<unsigned int>(mLoopStartMs),
                FMOD_TIMEUNIT_MS,
                static_cast<unsigned int>(mLoopEndMs),
                FMOD_TIMEUNIT_MS);
            mChannel->setLoopCount(-1);
        } else if (mLoopStartMs >= 0.0F) {
            mChannel->setLoopPoints(
                static_cast<unsigned int>(mLoopStartMs),
                FMOD_TIMEUNIT_MS,
                mLengthPcm - 1,
                FMOD_TIMEUNIT_PCM);
            mChannel->setLoopCount(-1);
        } else {
            mChannel->setLoopCount(0);
        }
        mLoopDirty = false;
    }

    const float nowMs = GetTimelineMs();
    if (mGain.Done()) {
        if (mStopAfterGain) {
            Stop();
            return true;
        }
    } else {
        mGain.Advance(nowMs - mLastPollMs);
    }
    if (!mMuteGain.Done()) {
        mMuteGain.Advance(nowMs - mLastPollMs);
    }
    mLastPollMs = nowMs;

    if (mState == kStatePlaying) {
        mChannel->setVolume(GetGain() * mMuteGain.mCurrent);
        if (mChannel->getPosition(&mPositionMs, FMOD_TIMEUNIT_MS) == FMOD_OK &&
            mPendingSeekMs != kNoPendingSeek && mPositionMs != mPendingSeekMs &&
            mChannel->setPosition(mPendingSeekMs, FMOD_TIMEUNIT_MS) == FMOD_OK) {
            mPositionMs = mPendingSeekMs;
            mPendingSeekMs = kNoPendingSeek;
        }
    }

    bool paused = false;
    if (mChannel->getPaused(&paused) == FMOD_OK && mPauseRequested != paused &&
        mChannel->setPaused(mPauseRequested) == FMOD_OK) {
        mState = mPauseRequested ? kStatePaused : kStatePlaying;
        if (mPauseRequested) {
            mPlayTimer.Pause();
        } else {
            mPlayTimer.Resume();
        }
    }
    if (mChannel != nullptr) {
        _UpdateWorldXfm();
    }
    return true;
}

// Reconstructed from eboot.elf at 0x269980. A bus that is still loading is
// retried a limited number of times before the stream plays unrouted.
void FmodAudioStreamGenerator::_TryStartChannel() {
    FMOD_OPENSTATE openState;
    unsigned int percentBuffered = 0;
    bool starving = false;
    bool diskBusy = false;
    mSound->getOpenState(&openState, &percentBuffered, &starving, &diskBusy);

    FMOD::ChannelGroup* channelGroup = nullptr;
    if (mStudioBus != nullptr) {
        const FMOD_RESULT result = mStudioBus->getChannelGroup(&channelGroup);
        if (result != FMOD_OK) {
            if (result == FMOD_ERR_STUDIO_NOT_LOADED && mBusRetries-- > 1) {
                return;
            }
            channelGroup = nullptr;
        }
    }
    if (openState != FMOD_OPENSTATE_READY) {
        return;
    }

    mSound->getLength(&mLengthMs, FMOD_TIMEUNIT_MS);
    mSound->getLength(&mLengthPcm, FMOD_TIMEUNIT_PCM);
    LowLevelSystemFor(mRenderTarget)->playSound(mSound, channelGroup, true, &mChannel);
    mChannel->setMode(FMOD_LOOP_NORMAL);
    mChannel->setLoopCount(0);
    mChannel->getFrequency(&mFrequency);
    mChannel->setVolume(GetGain() * mMuteGain.mCurrent);
    if (mPauseRequested) {
        mState = kStatePaused;
        return;
    }
    mChannel->setPaused(false);
    mState = kStatePlaying;
    mPlayTimer.Start();
}

// Reconstructed from eboot.elf at 0x269B40.
void FmodAudioStreamGenerator::_UpdateVolume() {
    mChannel->setVolume(GetGain() * mMuteGain.mCurrent);
}

// Reconstructed from eboot.elf at 0x269B70.
void FmodAudioStreamGenerator::_UpdateWorldXfm() {
    if (mChannel == nullptr || mEmitter == nullptr) {
        return;
    }
    FMOD_3D_ATTRIBUTES attributes;
    Convert(mEmitter->GetWorldXfm(), attributes);
    mChannel->set3DAttributes(&attributes.position, &attributes.velocity, nullptr);
}

// Reconstructed from eboot.elf at 0x269BF0.
void FmodAudioStreamGenerator::Pause() {
    mPauseRequested = true;
}

// Reconstructed from eboot.elf at 0x269C00.
void FmodAudioStreamGenerator::Continue() {
    mPauseRequested = false;
}

// Reconstructed from eboot.elf at 0x269C10. Time spent unpaused, from the
// play timer.
float FmodAudioStreamGenerator::GetElapsedMs() {
    return static_cast<float>(Hmx::Timer::CyclesToMs(mPlayTimer.Cycles()));
}

// Reconstructed from eboot.elf at 0x269C50.
float FmodAudioStreamGenerator::GetTimelineMs() {
    return static_cast<float>(mPositionMs);
}

// Reconstructed from eboot.elf at 0x26AA10.
float FmodAudioStreamGenerator::GetLengthMs() const {
    return static_cast<float>(mLengthMs);
}

// Reconstructed from eboot.elf at 0x269C60. The seek is applied on the next
// poll.
void FmodAudioStreamGenerator::SeekToMs(float ms) {
    mPendingSeekMs = static_cast<unsigned int>(ms);
}

// Reconstructed from eboot.elf at 0x269C70.
void FmodAudioStreamGenerator::SetSpeed(float speed, bool) {
    if (speed != mSpeed) {
        mSpeed = speed;
        mSpeedDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x269C90.
float FmodAudioStreamGenerator::GetSpeed(bool* changing) {
    if (changing != nullptr) {
        *changing = false;
    }
    return mSpeed;
}

// Reconstructed from eboot.elf at 0x269CB0.
void FmodAudioStreamGenerator::ClearLoop() {
    mLoopEndMs = -1.0F;
    mLoopStartMs = -1.0F;
    mLoopDirty = true;
}

// Reconstructed from eboot.elf at 0x269CD0.
void FmodAudioStreamGenerator::SetLoop(float endMs, float startMs) {
    mLoopEndMs = endMs;
    mLoopStartMs = startMs;
    mLoopDirty = true;
}

// Reconstructed from eboot.elf at 0x269CF0. The channel is stopped on the
// next poll.
void FmodAudioStreamGenerator::Stop() {
    mState = kStateStopping;
}

// Reconstructed from eboot.elf at 0x269D00.
void FmodAudioStreamGenerator::Kill() {
    if (mSound != nullptr) {
        Stop();
        mSound->release();
        mSound = nullptr;
    }
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x269D40. An immediate change while
// stopped or paused snaps the gain.
void FmodAudioStreamGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    mGain.SetDurationMs(std::fmax(fadeSecs * 1000.0F, kMinGainRampMs));
    mGain.Begin(gain);
    if (fadeSecs == 0.0F && mState != kStatePlaying) {
        mGain.Snap();
    }
    mStopAfterGain = option == kPostFadeStop;
}

// Reconstructed from eboot.elf at 0x269E70.
float FmodAudioStreamGenerator::GetGain() const {
    return mGain.mCurrent;
}

// Reconstructed from eboot.elf at 0x269E80.
void FmodAudioStreamGenerator::SetMute(bool mute, bool immediate) {
    mMuted = mute;
    mMuteGain.SetDurationMs(kMinGainRampMs);
    mMuteGain.Begin(mute ? 0.0F : 1.0F);
    if (immediate) {
        mMuteGain.Snap();
    }
}

// Reconstructed from eboot.elf at 0x269FA0.
bool FmodAudioStreamGenerator::GetMute() const {
    return mMuted;
}

// Reconstructed from eboot.elf at 0x269FB0.
void FmodAudioStreamGenerator::Release() {
    GeneratorPool::Release(*this);
    _CheckBusKnown(mStudioBus);
    mStudioBus = nullptr;
}

// Reconstructed from eboot.elf at 0x26AA20.
void FmodAudioStreamGenerator::_InitTypeId() {
    sTypeId = Symbol("FmodAudioStreamGenerator");
}

// Reconstructed from eboot.elf at 0x26AA70.
AudioGenerator* FmodAudioStreamGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x26AA90.
Symbol FmodAudioStreamGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x26A270.
AudioGenerator* FmodAudioStreamGeneratorManager::Play(const PlayArgs& args) {
    return PlayWithCallback(args, nullptr);
}

// Reconstructed from eboot.elf at 0x268CA0.
FmodAudioStreamGenerator* FmodAudioStreamGeneratorManager::PlayWithCallback(
    const PlayArgs& args, AudioBusCallable*) {
    ResourcePtr<FmodAudioStreamResource> resource = FmodAudioStreamResource::Find(args.mName);
    if (!resource || resource->Fail()) {
        return nullptr;
    }
    if (args.mFormat == kBufferedStreamFormat && args.mStreaming) {
        return nullptr;
    }
    return _AllocateAndSetUpGenerator(resource, args);
}

// Reconstructed from eboot.elf at 0x268D80.
FmodAudioStreamGenerator* FmodAudioStreamGeneratorManager::_AllocateAndSetUpGenerator(
    ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args) {
    AudioEmitter* emitter =
        args.mEmitter != nullptr ? args.mEmitter : theSoundManager.GetDefault2DEmitter();
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    auto* generator = GeneratorPool::Allocate<FmodAudioStreamGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    return generator->Setup(resource, args) ? generator : nullptr;
}

// Reconstructed from eboot.elf at 0x268C90.
void FmodAudioStreamGeneratorManager::Init() {
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x26A280.
int FmodAudioStreamGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x26A290.
Symbol FmodAudioStreamGeneratorManager::GetId() {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("FmodAudioStreamGeneratorManager");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x26A330.
Symbol FmodAudioStreamGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".mp3");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0x26A3D0.
AudioGenerator* FmodAudioStreamGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x26A450.
void FmodAudioStreamGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26A4C0.
void FmodAudioStreamGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26A530.
void FmodAudioStreamGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x26A680.
void FmodAudioStreamGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x26A690.
void FmodAudioStreamGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26A850.
bool FmodAudioStreamGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26A9E0.
FmodAudioStreamGeneratorManager::~FmodAudioStreamGeneratorManager() {}
