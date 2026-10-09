#include "audio/core/generators/AudioGenerator.h"

#include <utility>

#include "audio/core/components/AudioEmitterCom.h"
#include "audio/fmod/platform/orbis/FmodPlatform_PS4.h"

// The AudioGenerator defaults are inline in the map's build, emitted with the
// sound manager; this build keeps them at 0xE3E0 through 0xE5F0.

// Reconstructed from eboot.elf at 0xE3E0.
AudioGenerator::State AudioGenerator::GetState() {
    return mState;
}

// Reconstructed from eboot.elf at 0xE3F0.
float AudioGenerator::GetLengthMs() const {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0xE400.
void AudioGenerator::SetSpeed(float, bool) {}

// Reconstructed from eboot.elf at 0xE410.
float AudioGenerator::GetSpeed(bool*) {
    return 1.0F;
}

// Reconstructed from eboot.elf at 0xE420.
bool AudioGenerator::SetParameter(Symbol, float) {
    return false;
}

// Reconstructed from eboot.elf at 0xE430.
bool AudioGenerator::GetParameter(Symbol, float&) {
    return false;
}

// Reconstructed from eboot.elf at 0xE440.
void AudioGenerator::SetGain(float, float, PostFadeOption) {}

// Reconstructed from eboot.elf at 0xE450.
float AudioGenerator::GetGain() const {
    return 1.0F;
}

// Reconstructed from eboot.elf at 0xE460.
void AudioGenerator::SetMute(bool, bool) {}

// Reconstructed from eboot.elf at 0xE470.
bool AudioGenerator::GetMute() const {
    return false;
}

// Reconstructed from eboot.elf at 0xE480.
bool AudioGenerator::IsMusic() {
    return false;
}

// Reconstructed from eboot.elf at 0xE490.
bool AudioGenerator::IsInstrument() {
    return false;
}

// Reconstructed from eboot.elf at 0xE4A0.
bool AudioGenerator::IsDialog() {
    return false;
}

// Reconstructed from eboot.elf at 0xE4B0.
void AudioGenerator::Init(AudioGeneratorManager* manager, int index) {
    _InitTypeId();
    mManager = manager;
    mIndex = index;
}

// Reconstructed from eboot.elf at 0xE4E0. The pool node leaves the free list
// it is still on.
AudioGenerator::~AudioGenerator() {}

// Reconstructed from eboot.elf at 0xE5C0.
void* AudioGenerator::GetPluginData(const char*) {
    return nullptr;
}

// Reconstructed from eboot.elf at 0xE5D0.
void AudioGenerator::SetPlayScale(float) {}

// Reconstructed from eboot.elf at 0xE5E0.
float AudioGenerator::GetPlayScale() {
    return 1.0F;
}

// Reconstructed from eboot.elf at 0xE5F0.
float AudioGenerator::GetPrimaryStreamValue() {
    return 0.0F;
}

CritSec gGeneratorKillCritSec;

// Reconstructed from eboot.elf at 0x40500. The sound manager assigns the
// manager's index before the pool is built.
void AudioGeneratorManager::Init() {
    _SetManagerIndex(theSoundManager._RegisterGeneratorManager(this, GetResourceExt()));
    _InitGeneratorPool();
}

// Reconstructed from eboot.elf at 0x40540. The manager deletes itself once
// its pool could be freed.
bool AudioGeneratorManager::Destroy() {
    if (!_DeleteGeneratorPool()) {
        return false;
    }
    delete this;
    return true;
}

// Reconstructed from eboot.elf at 0x40570.
AudioGenerator* AudioGeneratorManager::Play(Symbol name, AudioEmitter* emitter, bool paused) {
    PlayArgs args;
    args.mName = name;
    args.mEmitter = emitter;
    args.mStartPaused = paused;
    return Play(args);
}

// Reconstructed from eboot.elf at 0x406C0.
AudioGenerator* AudioGeneratorManager::Prepare(Symbol name, AudioEmitter* emitter) {
    return Play(name, emitter, true);
}

// Reconstructed from eboot.elf at 0x406D0.
void AudioGenerator::KillLocked() {
    ScopedCritSec tracker(gGeneratorKillCritSec);
    Kill();
}

// Reconstructed from eboot.elf at 0x40720. The low 14 bits count the slot's
// reuses.
unsigned int AudioGenerator::GetNewHandle() {
    const unsigned int managerIndex = static_cast<unsigned int>(mManager->GetIndex());
    mHandle = ((mHandle + 1) & 0x3FFF) | ((static_cast<unsigned int>(mIndex) << 14) & 0xFFC000) |
        (managerIndex << 24) | kGeneratorHandleActive;
    return mHandle;
}

// Reconstructed from eboot.elf at 0x40770.
bool AudioGenerator::TryDeactivateHandle() {
    ScopedCritSecPtr tracker(&mManager->mCritSec);
    if (mRefCount > 0) {
        return false;
    }
    mHandle &= ~kGeneratorHandleActive;
    return true;
}

// Reconstructed from eboot.elf at 0x407C0.
void AudioGenerator::RegisterTempoListener(TempoListener* listener) {
    if (mEmitter != nullptr) {
        mEmitter->RegisterTempoListener(listener);
    }
}

// Reconstructed from eboot.elf at 0x407E0.
bool AudioGenerator::UnregisterTempoListener(TempoListener* listener) {
    if (mEmitter == nullptr) {
        return false;
    }
    return mEmitter->UnregisterTempoListener(listener);
}

// Reconstructed from eboot.elf at 0x40800.
bool GetEventParameterDefault(const char* event, const char* parameter, float* value) {
    if (gFmodPlatformInterface == nullptr) {
        return false;
    }
    return gFmodPlatformInterface->GetEventParameterDefault(event, parameter, value);
}

// Reconstructed from eboot.elf at 0x40830.
int GetEventParameterCount(const char* event) {
    if (gFmodPlatformInterface == nullptr) {
        return 0;
    }
    return gFmodPlatformInterface->GetEventParameterCount(event);
}

// Reconstructed from eboot.elf at 0x40860.
bool GetEventParameterByIndex(const char* event, int index, EventParameterInfo* info) {
    if (gFmodPlatformInterface == nullptr) {
        return false;
    }
    return gFmodPlatformInterface->GetEventParameterByIndex(event, index, info);
}

// Reconstructed from eboot.elf at 0x40890.
bool GetEventParameter(const char* event, const char* parameter, EventParameterInfo* info) {
    if (gFmodPlatformInterface == nullptr) {
        return false;
    }
    return gFmodPlatformInterface->GetEventParameter(event, parameter, info);
}

// Reconstructed from eboot.elf at 0x408C0.
eastl::vector<EnumValueDesc> GetPlayArgsRouteValues() {
    eastl::vector<EnumValueDesc> values;
    values.emplace_back(EnumValueDesc{
        PlayArgs::kRouteDefault, String("None"), String("Not routed through fmod event or bus.")});
    values.emplace_back(
        EnumValueDesc{PlayArgs::kRouteEvent, String("Event"), String("Routed through an fmod event.")});
    values.emplace_back(
        EnumValueDesc{PlayArgs::kRouteBus, String("Bus"), String("Routed through an fmod bus.")});
    return values;
}

// Reconstructed from eboot.elf at 0xE780, which 0x40AC0 jumps to. The free
// list unlinks its voices before the lock is destroyed.
AudioGeneratorManager::~AudioGeneratorManager() {}
