#include "audio/fmod/playback/FmodStudioSoundGenerator.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/fmod/io/FmodRecordingAudioRenderTarget.h"
#include "audio/fmod/playback/FmodGeneratorPool.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "math/transform/Transform.h"

namespace {

// Dialog requests belong to FmodDialogGeneratorManager. Name not in the
// reference map.
constexpr int kDialogFormat = 4;

FMOD::Studio::EventInstance* AsEventInstance(FMOD_STUDIO_EVENTINSTANCE* event) {
    return reinterpret_cast<FMOD::Studio::EventInstance*>(event);
}

}  // namespace

Symbol FmodStudioSoundGenerator::sTypeId("");

// Inlined into FmodStudioSoundGeneratorManager::Play at 0x270B20 and
// FmodDialogGeneratorManager::Play at 0x26EC60.
const char* MakeStudioEventPath(char (&buffer)[256], const char* name, bool allowSnapshot) {
    const std::size_t length = std::strlen(name);
    const bool hasEventScheme = allowSnapshot ? length >= 7 && name[5] == ':'
                                              : length >= 6 && name[5] == ':';
    const bool hasSnapshotScheme = allowSnapshot && length >= 10 && name[8] == ':';
    if (hasEventScheme || hasSnapshotScheme) {
        return name;
    }
    std::snprintf(buffer, sizeof(buffer), "event:/%s", name);
    return buffer;
}

// Reconstructed from eboot.elf at 0x26FB70.
FmodStudioSoundGenerator::FmodStudioSoundGenerator()
    : mEventInstance(nullptr), mLengthMs(0.0F), mEventDone(true) {}

// Reconstructed from eboot.elf at 0x2714D0.
FmodStudioSoundGenerator::~FmodStudioSoundGenerator() {}

// Reconstructed from eboot.elf at 0x26FC30. The event starts at once; a
// paused request starts it paused.
bool FmodStudioSoundGenerator::_Setup(
    const char* path,
    const PlayArgs& args,
    FMOD_STUDIO_EVENT_CALLBACK callback,
    void* userData) {
    FMOD::Studio::System* studio = FModSystemForTarget(mRenderTarget)->mStudioSystem;
    FMOD::Studio::EventDescription* description = nullptr;
    if (studio->getEvent(path, &description) != FMOD_OK) {
        return false;
    }
    int length = 0;
    description->getLength(&length);
    mLengthMs = static_cast<float>(length);
    description->createInstance(&mEventInstance);
    mEventInstance->setUserData(userData != nullptr ? userData : this);
    mLastPollMs = 0.0F;

    mGain.Begin(1.0F);
    mGain.Snap();
    mStopAfterGain = false;
    if (args.mHasInitialGain) {
        SetGain(
            std::pow(10.0F, args.mInitialGainDb * 0.05F),
            args.mInitialGainFadeSecs,
            static_cast<PostFadeOption>(args.mInitialGainPostFade));
    }
    mMuteGain.Begin(args.mStartMuted ? 0.0F : 1.0F);
    mMuted = args.mStartMuted;
    mMuteGain.Snap();

    if (args.mParameters != nullptr) {
        for (auto* parameter = args.mParameters->mBegin; parameter != args.mParameters->mEnd;
             ++parameter) {
            SetParameter(parameter->mName, parameter->mValue);
        }
    }
    mEventInstance->setCallback(
        callback != nullptr ? callback : _EventCallback, FMOD_STUDIO_EVENT_CALLBACK_ALL);
    if (args.mStartPaused) {
        mEventInstance->setPaused(true);
        mEventDone = false;
        mEventInstance->start();
        mState = kStatePaused;
    } else {
        if (mEmitter != nullptr) {
            FMOD_3D_ATTRIBUTES attributes;
            Convert(mEmitter->GetWorldXfm(), attributes);
            mEventInstance->set3DAttributes(&attributes);
        }
        mEventDone = false;
        mEventInstance->start();
        mState = kStatePlaying;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x26FF50. Destroy and stop callbacks allow
// the event to be released. The start callback binds the HMX plugins.
FMOD_RESULT FmodStudioSoundGenerator::_EventCallback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD_STUDIO_EVENTINSTANCE* event,
    void*) {
    FMOD::Studio::EventInstance* instance = AsEventInstance(event);
    FmodStudioSoundGenerator* generator = nullptr;
    if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED ||
        type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED) {
        instance->getUserData(reinterpret_cast<void**>(&generator));
        if (generator != nullptr) {
            generator->mEventDone = true;
        }
        return FMOD_OK;
    }
    if (type != FMOD_STUDIO_EVENT_CALLBACK_STARTED) {
        return FMOD_OK;
    }
    instance->getUserData(reinterpret_cast<void**>(&generator));
    if (generator->mState != kStateStopped && generator->mState != kStateStopping) {
        FMOD::ChannelGroup* channelGroup = nullptr;
        instance->getChannelGroup(&channelGroup);
        BindPluginsToGenerator(channelGroup, generator);
        // The emitter's mix group is queried; its channel group is inlined as
        // null in this build, so nothing is routed.
        generator->mEmitter->GetMixGroup();
    }
    generator->mEventDone = true;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2701D0.
void FmodStudioSoundGenerator::Pause() {
    mState = kStatePaused;
    mEventInstance->setPaused(true);
}

// Reconstructed from eboot.elf at 0x2701F0.
void FmodStudioSoundGenerator::Continue() {
    mState = kStatePlaying;
    mEventInstance->setPaused(false);
}

// Reconstructed from eboot.elf at 0x270210.
float FmodStudioSoundGenerator::GetElapsedMs() {
    int position = 0;
    mEventInstance->getTimelinePosition(&position);
    return static_cast<float>(position);
}

// Reconstructed from eboot.elf at 0x270260.
float FmodStudioSoundGenerator::GetTimelineMs() {
    int position = 0;
    mEventInstance->getTimelinePosition(&position);
    return static_cast<float>(position);
}

// Reconstructed from eboot.elf at 0x2714C0.
float FmodStudioSoundGenerator::GetLengthMs() const {
    return mLengthMs;
}

// Reconstructed from eboot.elf at 0x2702B0.
void FmodStudioSoundGenerator::SeekToMs(float ms) {
    mEventInstance->setTimelinePosition(static_cast<int>(ms));
}

// Reconstructed from eboot.elf at 0x2702C0. A stopped event is released; a
// stopping dialog event is cut immediately. The gain ramps advance against
// the timeline position.
bool FmodStudioSoundGenerator::Poll() {
    FMOD_STUDIO_PLAYBACK_STATE playbackState;
    if (mEventInstance == nullptr ||
        mEventInstance->getPlaybackState(&playbackState) == FMOD_ERR_INVALID_HANDLE) {
        mState = kStateStopped;
        mEventDone = true;
        return false;
    }
    bool stopping = false;
    if (playbackState == FMOD_STUDIO_PLAYBACK_STOPPING && IsDialog()) {
        mEventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        stopping = true;
    }
    if (!stopping && playbackState == FMOD_STUDIO_PLAYBACK_STOPPED) {
        mEventInstance->release();
        mState = kStateStopped;
        mEventDone = true;
        return false;
    }

    if (mEmitter != nullptr) {
        FMOD_3D_ATTRIBUTES attributes;
        Convert(mEmitter->GetWorldXfm(), attributes);
        mEventInstance->set3DAttributes(&attributes);
    }
    const float nowMs = GetElapsedMs();
    if (nowMs > mLastPollMs) {
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
    }
    mLastPollMs = nowMs;
    if (mState == kStatePlaying && mEventInstance != nullptr) {
        mEventInstance->setVolume(GetGain() * mMuteGain.mCurrent);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x270620. A paused event stops at once.
void FmodStudioSoundGenerator::Stop() {
    if (mEventInstance == nullptr) {
        return;
    }
    mEventInstance->stop(
        mState == kStatePaused ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT);
    mState = kStateStopping;
}

// Reconstructed from eboot.elf at 0x270660.
void FmodStudioSoundGenerator::_ReleaseEvent() {
    if (mEventInstance == nullptr) {
        return;
    }
    mEventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    mEventInstance->release();
    mEventInstance = nullptr;
}

// Reconstructed from eboot.elf at 0x2706A0. Polls and flushes Studio until
// the event's stop callback arrives.
void FmodStudioSoundGenerator::Kill() {
    if (mEventInstance == nullptr) {
        return;
    }
    mState = kStateStopping;
    mEventInstance->setCallback(nullptr, FMOD_STUDIO_EVENT_CALLBACK_ALL);
    mEventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    while (!mEventDone) {
        if (Poll()) {
            FModSystem::Get()->mStudioSystem->flushCommands();
        }
    }
    mState = kStateStopped;
    mEventInstance->release();
    mEventInstance = nullptr;
    mLengthMs = 0.0F;
}

// Reconstructed from eboot.elf at 0x270750.
void FmodStudioSoundGenerator::Release() {
    FmodGeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x2707D0.
bool FmodStudioSoundGenerator::SetParameter(Symbol name, float value) {
    if (mEventInstance == nullptr || mState == kStateStopping) {
        return false;
    }
    return mEventInstance->setParameterValue(name.Str(), value) == FMOD_OK;
}

// Reconstructed from eboot.elf at 0x270800.
bool FmodStudioSoundGenerator::GetParameter(Symbol name, float& value) {
    if (mEventInstance == nullptr || mState == kStateStopping) {
        return false;
    }
    FMOD::Studio::ParameterInstance* parameter = nullptr;
    return mEventInstance->getParameter(name.Str(), &parameter) == FMOD_OK &&
        parameter->getValue(&value) == FMOD_OK;
}

// Reconstructed from eboot.elf at 0x270880. An immediate change outside
// playback snaps the gain and applies it to the event.
void FmodStudioSoundGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    mGain.SetDurationMs(std::fmax(fadeSecs * 1000.0F, kMinGainRampMs));
    mGain.Begin(gain);
    if (fadeSecs == 0.0F && mState != kStatePlaying) {
        mGain.Snap();
        if (mEventInstance != nullptr) {
            mEventInstance->setVolume(GetGain() * mMuteGain.mCurrent);
        }
    }
    mStopAfterGain = option == kPostFadeStop;
}

// Reconstructed from eboot.elf at 0x2709A0.
float FmodStudioSoundGenerator::GetGain() const {
    return mGain.mCurrent;
}

// Reconstructed from eboot.elf at 0x2709B0.
void FmodStudioSoundGenerator::SetMute(bool mute, bool immediate) {
    mMuted = mute;
    mMuteGain.SetDurationMs(kMinGainRampMs);
    mMuteGain.Begin(mute ? 0.0F : 1.0F);
    if (immediate) {
        mMuteGain.Snap();
    }
    if (mState != kStatePlaying && mEventInstance != nullptr) {
        mEventInstance->setVolume(GetGain() * mMuteGain.mCurrent);
    }
}

// Reconstructed from eboot.elf at 0x270B10.
bool FmodStudioSoundGenerator::GetMute() const {
    return mMuted;
}

// Reconstructed from eboot.elf at 0x2715B0.
void FmodStudioSoundGenerator::_InitTypeId() {
    sTypeId = Symbol("FmodStudioSoundGenerator");
}

// Reconstructed from eboot.elf at 0x271600.
AudioGenerator* FmodStudioSoundGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x271620.
Symbol FmodStudioSoundGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x270B20. The event must exist in a loaded
// bank; a voice whose setup fails is not returned to the pool.
AudioGenerator* FmodStudioSoundGeneratorManager::Play(const PlayArgs& args) {
    if (args.mFormat == kDialogFormat) {
        return nullptr;
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    if (target == nullptr) {
        return nullptr;
    }
    FModSystem* system = FModSystemForTarget(target);
    if (system == nullptr || system->mStudioSystem == nullptr ||
        system->mLowLevelSystem == nullptr) {
        return nullptr;
    }
    char buffer[256];
    const char* path = MakeStudioEventPath(buffer, args.mName.Str(), true);
    FMOD_GUID id;
    if (system->mStudioSystem->lookupID(path, &id) != FMOD_OK) {
        return nullptr;
    }
    AudioEmitterCom* emitter =
        args.mEmitter != nullptr ? args.mEmitter : theSoundManager.GetDefault2DEmitter();
    auto* generator = FmodGeneratorPool::Allocate<FmodStudioSoundGenerator>(*this, target, emitter);
    if (generator != nullptr && generator->_Setup(path, args, nullptr, nullptr)) {
        return generator;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x270D90.
int FmodStudioSoundGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x270DA0.
Symbol FmodStudioSoundGeneratorManager::GetId() {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("FmodStudioSoundGeneratorManager");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x270E40.
Symbol FmodStudioSoundGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".bank");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0x270EE0.
AudioGenerator* FmodStudioSoundGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return FmodGeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x270F50.
void FmodStudioSoundGeneratorManager::SendStopToAllGenerators() {
    FmodGeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x270FC0.
void FmodStudioSoundGeneratorManager::SendKillToAllGenerators() {
    FmodGeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x271030.
void FmodStudioSoundGeneratorManager::GetActiveHandles(void* handles) {
    FmodGeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x271180.
void FmodStudioSoundGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x271190.
void FmodStudioSoundGeneratorManager::_InitGeneratorPool() {
    FmodGeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x271350.
bool FmodStudioSoundGeneratorManager::_DeleteGeneratorPool() {
    return FmodGeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x271490.
FmodStudioSoundGeneratorManager::~FmodStudioSoundGeneratorManager() {}
