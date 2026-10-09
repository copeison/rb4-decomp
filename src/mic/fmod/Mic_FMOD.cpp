#include "mic/fmod/Mic_FMOD.h"

#include <cstring>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "mic/core/MicHwManager.h"
#include "mic/fmod/MicHwManager_FMOD.h"
#include "os/threading/CritSec.h"

namespace {

constexpr int kNoDriver = -1;
// Mic type reported by FMOD microphones. Name not in the reference map.
constexpr int kMicTypeFmod = 1;
// Status of a mic bound to hardware. Name not in the reference map.
constexpr int kMicStatusAttached = 2;
constexpr int kDefaultSampleRate = 48000;

// The record buffer holds half a second of 16-bit stereo; the rings take
// the left channel.
constexpr int kRecordChannels = 2;
constexpr int kRecordFrameBytes = 4;
// Playback latency targets in milliseconds, and the playback rate change in
// percent. Names not in the reference map.
constexpr int kMinLatencyMs = 30;
constexpr int kMaxLatencyMs = 150;
constexpr int kLatencyToleranceDivisor = 200;
constexpr int kRateAdjustDivisor = 100;
constexpr float kLatencySmoothing = 0.97F;
constexpr float kLatencyWeight = 0.03F;
constexpr float kCatchUpGain = 4.0F;

FMOD::System* LowLevelSystem() {
    return FModSystem::Get()->mLowLevelSystem;
}

// Detaches a mic whose driver failed and flags the mic list. Inlined at each
// failure in _Poll and MicThreadPoll. Name not in the reference map.
void DetachMic(Mic_FMOD& mic) {
    mic._HandleMicRemoval();
    mic.mDriverName = Symbol("");
    mic.mDriver = kNoDriver;
    gMicHwManager.MarkMicsChanged();
}

}  // namespace

Mic_FMOD::Mic_FMOD(int index)
    : mDriver(kNoDriver),
      mIndex(index),
      mDriverName(""),
      mSound(nullptr),
      mChannel(nullptr),
      mSoundLengthBytes(0),
      mSoundLength(0),
      mLastRecordPos(0),
      mRecordedFrames(0),
      mMinLatency(0),
      mTargetLatency(0),
      mMaxLatency(4410),
      mLatencyTolerance(0),
      mMinRecordAdvance(0xFFFFFFFF),
      mSmoothedLatency(0.0F),
      mRateState(kRateNormal),
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

// Reconstructed from eboot.elf at 0x27BB20. A Studio bus route plays the
// mic through the bus's channel group; an event route plays it through a
// new event instance.
void Mic_FMOD::StartPlayback(const PlayArgs& args) {
    if (mDriver == kNoDriver || mSound != nullptr) {
        return;
    }
    FMOD::Studio::System* studio = FModSystem::Get()->mStudioSystem;
    if (args.mRoute == PlayArgs::kRouteBus) {
        FMOD::Studio::Bus* bus;
        if (studio->getBus(args.mRoutePath.Str(), &bus) == FMOD_OK) {
            bus->getChannelGroup(&mPlaybackGroup);
        }
    } else if (args.mRoute == PlayArgs::kRouteEvent) {
        FMOD::Studio::EventDescription* description;
        if (studio->getEvent(args.mRoutePath.Str(), &description) == FMOD_OK &&
            description->createInstance(&mEventInstance) == FMOD_OK) {
            if (args.mParameters != nullptr) {
                for (auto* parameter = args.mParameters->begin(); parameter != args.mParameters->end();
                     ++parameter) {
                    mEventInstance->setParameterValue(parameter->mName.Str(), parameter->mValue);
                }
            }
            mEventInstance->start();
        }
    }
    Start();
}

// Reconstructed from eboot.elf at 0x27BC50. Records into a looping user
// sound and resets the playback latency tracking.
void Mic_FMOD::Start() {
    if (mDriver == kNoDriver || mSound != nullptr) {
        return;
    }
    FMOD::System* lowLevel = LowLevelSystem();
    FMOD_CREATESOUNDEXINFO info = {};
    info.cbsize = sizeof(info);
    info.numchannels = kRecordChannels;
    info.defaultfrequency = mSampleRate;
    info.format = FMOD_SOUND_FORMAT_PCM16;
    info.length = 2 * mSampleRate;
    ScopedCritSecPtr tracker(&mCritSec);
    lowLevel->createSound(nullptr, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &info, &mSound);
    mSound->getLength(&mSoundLengthBytes, FMOD_TIMEUNIT_PCMBYTES);
    mSound->getLength(&mSoundLength, FMOD_TIMEUNIT_PCM);
    if (lowLevel->recordStart(mDriver, mSound, true) != FMOD_OK) {
        if (mSound != nullptr) {
            mSound->release();
            mSound = nullptr;
        }
        return;
    }
    mLastRecordPos = 0;
    mRecordedFrames = 0;
    int sampleRate = mSampleRate;
    mMinLatency = kMinLatencyMs * sampleRate / 1000;
    mTargetLatency = mMinLatency;
    mMaxLatency = kMaxLatencyMs * sampleRate / 1000;
    mLatencyTolerance = sampleRate / kLatencyToleranceDivisor;
    mMinRecordAdvance = 0xFFFFFFFF;
    mSmoothedLatency = 0.0F;
    mRateState = kRateNormal;
    mLastReadPos = -1;
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

// Reconstructed from eboot.elf at 0x27BF80.
void Mic_FMOD::_Poll() {
    ScopedCritSecPtr tracker(&mCritSec);
    if (mSound == nullptr) {
        return;
    }
    unsigned int recordPos = 0;
    if (LowLevelSystem()->getRecordPosition(mDriver, &recordPos) != FMOD_OK) {
        DetachMic(*this);
        return;
    }
    if (mLastReadPos < 0) {
        mLastReadPos = static_cast<int>(recordPos);
        return;
    }
    int frames = static_cast<int>(recordPos) - mLastReadPos;
    if (frames < 0) {
        frames += mSoundLength;
    }
    void* data1;
    void* data2;
    unsigned int bytes1;
    unsigned int bytes2;
    if (mSound->lock(
            mLastReadPos * kRecordFrameBytes, frames * kRecordFrameBytes, &data1, &data2, &bytes1, &bytes2) !=
        FMOD_OK) {
        DetachMic(*this);
        return;
    }
    int frames1 = bytes1 / kRecordFrameBytes;
    int frames2 = bytes2 / kRecordFrameBytes;
    mRecentBuffer.Write(static_cast<short*>(data1), frames1, kRecordChannels);
    mOverflowCount = mContinuousBuffer.Write(static_cast<short*>(data1), frames1, kRecordChannels);
    if (frames2 != 0) {
        mRecentBuffer.Write(static_cast<short*>(data2), frames2, kRecordChannels);
        mOverflowCount += mContinuousBuffer.Write(static_cast<short*>(data2), frames2, kRecordChannels);
    }
    mSound->unlock(data1, data2, bytes1, bytes2);
    int length = static_cast<int>(mSoundLength);
    int position = static_cast<int>(recordPos) + length;
    do {
        position -= length;
    } while (position >= length);
    mLastReadPos = position;
}

// Reconstructed from eboot.elf at 0x27C300. Playback of the record buffer
// starts once the target latency is recorded. Each pass then measures how
// far playback trails recording and changes the playback rate by 1% to
// keep the smoothed latency near the target; past the maximum it speeds up
// in proportion to the excess.
void Mic_FMOD::MicThreadPoll() {
    ScopedCritSecPtr tracker(&mCritSec);
    if (mSound == nullptr) {
        return;
    }
    FMOD::System* lowLevel = LowLevelSystem();
    unsigned int recordPos = 0;
    if (lowLevel->getRecordPosition(mDriver, &recordPos) != FMOD_OK) {
        DetachMic(*this);
        return;
    }
    unsigned int advance = recordPos;
    if (recordPos < mLastRecordPos) {
        advance = recordPos + mSoundLength;
    }
    advance -= mLastRecordPos;
    mLastRecordPos = recordPos;
    mRecordedFrames += advance;

    if (mRecordedFrames >= mTargetLatency && mChannel == nullptr) {
        FMOD::ChannelGroup* group = mPlaybackGroup;
        if (group == nullptr) {
            if (mEventInstance != nullptr) {
                if (mEventInstance->getChannelGroup(&group) == FMOD_ERR_STUDIO_NOT_LOADED) {
                    return;
                }
            } else {
                group = gMicHwManagerFMOD.GetBusChannelGroup(mIndex);
            }
        }
        if (lowLevel->playSound(mSound, group, false, &mChannel) != FMOD_OK) {
            DetachMic(*this);
            return;
        }
        mChannel->setVolume(mVolume);
        mChannel->setMute(mMuted);
    }
    if (advance == 0 || mChannel == nullptr) {
        return;
    }
    if (advance < mMinRecordAdvance && advance < 2 * mMinLatency) {
        mMinRecordAdvance = advance;
        mTargetLatency = advance > mMinLatency ? advance : mMinLatency;
    }

    unsigned int playPos = 0;
    if (mChannel->getPosition(&playPos, FMOD_TIMEUNIT_PCM) != FMOD_OK) {
        DetachMic(*this);
        return;
    }
    // Playback just ahead of recording counts as a negative latency.
    int sampleRate = mSampleRate;
    unsigned int window = kMaxLatencyMs * sampleRate / 1000;
    int latency = static_cast<int>(recordPos - playPos);
    if (recordPos < playPos) {
        if (playPos - recordPos >= window) {
            latency = static_cast<int>(recordPos - playPos + mSoundLength);
        }
    } else if (playPos + mSoundLength - recordPos < window) {
        latency = static_cast<int>(recordPos - (playPos + mSoundLength));
    }

    float rate = static_cast<float>(sampleRate);
    float smoothed =
        mSmoothedLatency * kLatencySmoothing + static_cast<float>(latency) * kLatencyWeight;
    mSmoothedLatency = smoothed;
    float frequency = rate;
    if (latency > mMaxLatency) {
        mSmoothedLatency = static_cast<float>(latency);
        mRateState = kRateCatchUp;
        float minimum = static_cast<float>(sampleRate + sampleRate / kRateAdjustDivisor);
        float catchUp =
            static_cast<float>(static_cast<unsigned int>(latency) - mTargetLatency) * kCatchUpGain + rate;
        frequency = minimum > catchUp ? minimum : catchUp;
    } else {
        if (mRateState == kRateCatchUp) {
            mRateState = kRateNormal;
        }
        bool slow;
        bool fast = false;
        if (mRateState == kRateSlow) {
            slow = smoothed < static_cast<float>(mTargetLatency);
        } else {
            slow = smoothed < static_cast<float>(mTargetLatency - mLatencyTolerance);
            unsigned int fastThreshold =
                mRateState == kRateFast ? mTargetLatency : mTargetLatency + mLatencyTolerance;
            fast = !slow && smoothed > static_cast<float>(fastThreshold);
        }
        if (slow) {
            mRateState = kRateSlow;
            frequency = static_cast<float>(sampleRate - sampleRate / kRateAdjustDivisor);
        } else if (fast) {
            mRateState = kRateFast;
            frequency = static_cast<float>(sampleRate + sampleRate / kRateAdjustDivisor);
        } else {
            mRateState = kRateNormal;
        }
    }
    if (frequency != mFrequency) {
        mFrequency = frequency;
        mChannel->setFrequency(frequency);
    }
}
