#include "mic/fmod/Mic_FMOD.h"

#include <cstring>

#include "audio/fmod/system/FmodPlatform.h"
#include "os/threading/CritSec.h"

namespace {

constexpr int kNoDriver = -1;
// Mic type reported by FMOD microphones. Name not in the reference map.
constexpr int kMicTypeFmod = 1;
// Status of a mic bound to hardware. Name not in the reference map.
constexpr int kMicStatusAttached = 2;
constexpr int kDefaultSampleRate = 48000;

FMOD::System* LowLevelSystem() {
    return FModSystem::Get()->mLowLevelSystem;
}

}  // namespace

Mic_FMOD::Mic_FMOD(int index)
    : mDriver(kNoDriver),
      mIndex(index),
      mDriverName(""),
      mSound(nullptr),
      mChannel(nullptr),
      mSoundLength(0),
      mUnknown16660(),
      mUnknown16680(4410),
      mUnknown16684(0),
      mUnknown16688(-1),
      mUnknown16692(0),
      mUnknown16696(0),
      mEventInstance(nullptr),
      mPlaybackGroup(nullptr) {
    mSampleRate = kDefaultSampleRate;
    mFrequency = static_cast<float>(kDefaultSampleRate);
}

// Reconstructed from eboot.elf at 0x27B680.
Mic_FMOD::~Mic_FMOD() {
    ScopedCritSecPtr tracker(&mCritSec);
    if (mSound != nullptr) {
        mSound->release();
        mSound = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x27C7C0.
int Mic_FMOD::GetStatus() const {
    return mDriver != kNoDriver ? kMicStatusAttached : 0;
}

// Reconstructed from eboot.elf at 0x27C7D0.
int Mic_FMOD::GetType() const {
    return kMicTypeFmod;
}

// Reconstructed from eboot.elf at 0x27C7E0.
bool Mic_FMOD::IsRunning() const {
    return mSound != nullptr;
}

// Reconstructed from eboot.elf at 0x27C7F0.
Symbol Mic_FMOD::GetName() const {
    return mDriverName;
}

// Reconstructed from eboot.elf at 0x27B780.
bool Mic_FMOD::AttachToHardware(int driver, Symbol name) {
    char driverName[256];
    FMOD_GUID guid;
    int sampleRate = 0;
    FMOD_SPEAKERMODE speakerMode;
    int channels = 0;
    FMOD_DRIVER_STATE state;
    if (LowLevelSystem()->getRecordDriverInfo(
            driver, driverName, sizeof(driverName), &guid, &sampleRate, &speakerMode,
            &channels, &state) != FMOD_OK) {
        return false;
    }
    if (std::strcmp(name.Str(), driverName) != 0) {
        return false;
    }
    mDriverName = name;
    mDriver = driver;
    mSampleRate = sampleRate;
    mFrequency = static_cast<float>(sampleRate);
    return true;
}

// Reconstructed from eboot.elf at 0x27B880. The driver is found by name; a
// missing or disconnected driver releases the mic.
bool Mic_FMOD::CheckDeviceStillConnected() {
    ScopedCritSecPtr tracker(&mCritSec);
    if (std::strcmp(mDriverName.Str(), "") == 0) {
        return false;
    }

    FMOD::System* lowLevel = LowLevelSystem();
    int numDrivers = 0;
    int numConnected = 0;
    lowLevel->getRecordNumDrivers(&numDrivers, &numConnected);
    for (int driver = 0; driver < numDrivers; ++driver) {
        char driverName[256];
        FMOD_GUID guid;
        int sampleRate = 0;
        FMOD_SPEAKERMODE speakerMode;
        int channels = 0;
        FMOD_DRIVER_STATE state = 0;
        if (lowLevel->getRecordDriverInfo(
                driver, driverName, sizeof(driverName), &guid, &sampleRate, &speakerMode,
                &channels, &state) != FMOD_OK ||
            std::strcmp(mDriverName.Str(), driverName) != 0) {
            continue;
        }
        if ((state & FMOD_DRIVER_STATE_CONNECTED) != 0) {
            return true;
        }
        break;
    }

    _HandleMicRemoval();
    mDriverName = Symbol("");
    mDriver = kNoDriver;
    return false;
}

// Reconstructed from eboot.elf at 0x27BE70.
void Mic_FMOD::Stop() {
    ScopedCritSecPtr tracker(&mCritSec);
    if (mDriver == kNoDriver || mSound == nullptr) {
        return;
    }
    LowLevelSystem()->recordStop(mDriver);
    mSound->release();
    mSound = nullptr;
    mChannel = nullptr;
    if (mEventInstance != nullptr) {
        mEventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        mEventInstance->release();
        mEventInstance = nullptr;
    }
    mPlaybackGroup = nullptr;
}

// Reconstructed from eboot.elf at 0x27BF40.
void Mic_FMOD::_ApplyVolume() {
    if (mChannel != nullptr) {
        mChannel->setVolume(mVolume);
    }
}

// Reconstructed from eboot.elf at 0x27BF60.
void Mic_FMOD::_ApplyMute() {
    if (mChannel != nullptr) {
        mChannel->setMute(mMuted);
    }
}
