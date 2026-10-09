#include "audio/fmod/playback/FmodDialogGenerator.h"

#include "audio/fmod/io/FmodRecordingAudioRenderTarget.h"
#include "audio/fmod/playback/FmodGeneratorPool.h"
#include "audio/fmod/system/FmodPlatform.h"

namespace {

// Only dialog requests reach this manager. Name not in the reference map.
constexpr int kDialogFormat = 4;
// Length reported until the first programmer sound is measured: one hour.
constexpr float kUnknownDialogLengthMs = 3600000.0F;
constexpr FMOD_MODE kProgrammerSoundMode =
    FMOD_NONBLOCKING | FMOD_ACCURATETIME | FMOD_CREATECOMPRESSEDSAMPLE;

}  // namespace

Symbol FmodDialogGenerator::sTypeId("");

// Reconstructed from eboot.elf at 0x26E990. The embedded studio generator
// plays the event with this generator as the callback's user data.
bool FmodDialogGenerator::Setup(const char* path, const PlayArgs& args) {
    if (args.mFormat != kDialogFormat) {
        return false;
    }
    DialogGenerator::Setup(path, args);
    mLengthMs = kUnknownDialogLengthMs;
    mStudio.mEmitter = mEmitter;
    mStudio.mRenderTarget = mRenderTarget;
    return mStudio._Setup(path, args, _ProgrammerSoundCallback, this);
}

// Reconstructed from eboot.elf at 0x26EA10. Programmer sounds are created from
// the Studio sound table and their names are passed to the dialog sink.
FMOD_RESULT FmodDialogGenerator::_ProgrammerSoundCallback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD_STUDIO_EVENTINSTANCE* event,
    void* parameters) {
    FmodDialogGenerator* generator = nullptr;
    reinterpret_cast<FMOD::Studio::EventInstance*>(event)->getUserData(
        reinterpret_cast<void**>(&generator));
    const State state = generator->GetState();
    if (state == kStateStopped || state == kStateStopping) {
        generator->mStudio.mEventDone = true;
        return FMOD_OK;
    }

    auto* properties = static_cast<FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*>(parameters);
    if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND) {
        return properties->sound->release();
    }
    if (type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED) {
        if (parameters != nullptr) {
            unsigned int length = 0;
            static_cast<FMOD::Sound*>(parameters)->getLength(&length, FMOD_TIMEUNIT_MS);
            generator->mLengthMs = static_cast<float>(length);
        }
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_STARTED) {
        generator->mStudio.mEmitter->GetMixGroup();
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND) {
        auto* system = static_cast<FModSystem*>(generator->mStudio.mRenderTarget);
        FMOD_STUDIO_SOUND_INFO info;
        const FMOD_RESULT infoResult =
            system->mStudioSystem->getSoundInfo(properties->name, &info);
        if (infoResult != FMOD_OK) {
            return infoResult;
        }
        FMOD::Sound* sound = nullptr;
        const FMOD_RESULT soundResult = system->mLowLevelSystem->createSound(
            info.name_or_data, info.mode | kProgrammerSoundMode, &info.exinfo, &sound);
        if (soundResult != FMOD_OK) {
            return soundResult;
        }
        properties->sound = sound;
        properties->subsoundIndex = info.subsoundindex;
        generator->NotifySoundCreated(properties->name);
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x26EBE0.
void FmodDialogGenerator::Release() {
    mStudio._ReleaseEvent();
    FmodGeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x26F6E0.
void FmodDialogGenerator::Pause() {
    mStudio.FmodStudioSoundGenerator::Pause();
}

// Reconstructed from eboot.elf at 0x26F6F0.
void FmodDialogGenerator::Continue() {
    mStudio.FmodStudioSoundGenerator::Continue();
}

// Reconstructed from eboot.elf at 0x26F700.
void FmodDialogGenerator::Stop() {
    mStudio.FmodStudioSoundGenerator::Stop();
}

// Reconstructed from eboot.elf at 0x26F710.
AudioGenerator::State FmodDialogGenerator::GetState() {
    return mStudio.mState;
}

// Reconstructed from eboot.elf at 0x26F720.
float FmodDialogGenerator::GetElapsedMs() {
    return mStudio.FmodStudioSoundGenerator::GetElapsedMs();
}

// Reconstructed from eboot.elf at 0x26F730.
float FmodDialogGenerator::GetTimelineMs() {
    return mStudio.FmodStudioSoundGenerator::GetTimelineMs();
}

// Reconstructed from eboot.elf at 0x26F740.
float FmodDialogGenerator::GetLengthMs() const {
    return mLengthMs;
}

// Reconstructed from eboot.elf at 0x26F750.
void FmodDialogGenerator::SeekToMs(float ms) {
    mStudio.FmodStudioSoundGenerator::SeekToMs(ms);
}

// Reconstructed from eboot.elf at 0x26F760.
bool FmodDialogGenerator::SetParameter(Symbol name, float value) {
    return mStudio.FmodStudioSoundGenerator::SetParameter(name, value);
}

// Reconstructed from eboot.elf at 0x26F770.
bool FmodDialogGenerator::GetParameter(Symbol name, float& value) {
    return mStudio.FmodStudioSoundGenerator::GetParameter(name, value);
}

// Reconstructed from eboot.elf at 0x26F780.
void FmodDialogGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    mStudio.FmodStudioSoundGenerator::SetGain(gain, fadeSecs, option);
}

// Reconstructed from eboot.elf at 0x26F790.
float FmodDialogGenerator::GetGain() const {
    return mStudio.FmodStudioSoundGenerator::GetGain();
}

// Reconstructed from eboot.elf at 0x26F7A0.
void FmodDialogGenerator::SetMute(bool mute, bool immediate) {
    mStudio.FmodStudioSoundGenerator::SetMute(mute, immediate);
}

// Reconstructed from eboot.elf at 0x26F7C0.
bool FmodDialogGenerator::GetMute() const {
    return mStudio.FmodStudioSoundGenerator::GetMute();
}

// Reconstructed from eboot.elf at 0x26F7D0.
bool FmodDialogGenerator::IsDialog() {
    return true;
}

// Reconstructed from eboot.elf at 0x26F7E0.
FmodDialogGenerator::~FmodDialogGenerator() {}

// Reconstructed from eboot.elf at 0x26FAB0.
bool FmodDialogGenerator::Poll() {
    return mStudio.FmodStudioSoundGenerator::Poll();
}

// Reconstructed from eboot.elf at 0x26FAC0.
void FmodDialogGenerator::_InitTypeId() {
    sTypeId = Symbol("FmodDialogGenerator");
}

// Reconstructed from eboot.elf at 0x26FB10.
void FmodDialogGenerator::Kill() {
    mStudio.KillLocked();
}

// Reconstructed from eboot.elf at 0x26FB20.
AudioGenerator* FmodDialogGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x26FB40.
Symbol FmodDialogGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x26EC60.
AudioGenerator* FmodDialogGeneratorManager::Play(const PlayArgs& args) {
    if (args.mFormat != kDialogFormat) {
        return nullptr;
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    FMOD::Studio::System* studio = FModSystemForTarget(target)->mStudioSystem;
    char buffer[256];
    const char* path = MakeStudioEventPath(buffer, args.mName.Str(), false);
    FMOD_GUID id;
    if (studio->lookupID(path, &id) != FMOD_OK) {
        return nullptr;
    }
    AudioEmitterCom* emitter =
        args.mEmitter != nullptr ? args.mEmitter : gSoundManager.GetDefault2DEmitter();
    auto* generator = FmodGeneratorPool::Allocate<FmodDialogGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    return generator->Setup(path, args) ? generator : nullptr;
}

// Reconstructed from eboot.elf at 0x26EE80.
int FmodDialogGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x26EE90.
Symbol FmodDialogGeneratorManager::GetId() {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("FmodDialogGeneratorManager");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x26EF30.
Symbol FmodDialogGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".bank");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0x26EFD0.
AudioGenerator* FmodDialogGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return FmodGeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x26F040.
void FmodDialogGeneratorManager::SendStopToAllGenerators() {
    FmodGeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26F0B0.
void FmodDialogGeneratorManager::SendKillToAllGenerators() {
    FmodGeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26F120.
void FmodDialogGeneratorManager::GetActiveHandles(void* handles) {
    FmodGeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x26F270.
void FmodDialogGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x26F280.
void FmodDialogGeneratorManager::_InitGeneratorPool() {
    FmodGeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26F420.
bool FmodDialogGeneratorManager::_DeleteGeneratorPool() {
    return FmodGeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26F6B0.
FmodDialogGeneratorManager::~FmodDialogGeneratorManager() {}
