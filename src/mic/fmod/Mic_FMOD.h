#pragma once

#include <cstddef>

#include "audio/core/system/Audio.h"
#include "audio/fmod/api/fmod_api.h"
#include "mic/core/Mic.h"
#include "os/threading/CritSec.h"

// Microphone recorded through an FMOD record driver. The vtable is at
// 0x18F1208; the object is 16720 bytes.
class Mic_FMOD : public Mic {
public:
    // Reconstructed from eboot.elf at 0x27B3E0. The map has Mic_FMOD(); this
    // build passes the slot index.
    explicit Mic_FMOD(int index);
    ~Mic_FMOD() override;                 // slots 0-1: 0x27B680, 0x27B760

    int GetStatus() const override;       // slot 2: 0x27C7C0
    int GetType() const override;         // slot 3: 0x27C7D0
    bool IsRunning() const override;      // slot 4: 0x27C7E0
    void MicThreadPoll() override;        // slot 7: 0x27C300
    Symbol GetName() const override;      // slot 9: 0x27C7F0
    void Start() override;                // slot 10: 0x27BC50
    void StartPlayback(const PlayArgs& args) override;  // slot 11: 0x27BB20
    void Stop() override;                 // slot 12: 0x27BE70
    void _ApplyVolume() override;         // slot 13: 0x27BF40
    void _ApplyMute() override;           // slot 14: 0x27BF60
    // Slot 15 at 0x27BF80: copies the newly recorded frames into the rings
    // and counts the continuous ring's overflow.
    void _Poll() override;

    // Binds the mic to a record driver whose name still matches. Reconstructed
    // from eboot.elf at 0x27B780.
    bool AttachToHardware(int driver, Symbol name);
    // Reconstructed from eboot.elf at 0x27B880. A missing or disconnected
    // driver detaches the mic.
    bool CheckDeviceStillConnected();

    // Playback rate states of MicThreadPoll. Names not in the reference
    // map.
    enum RateState : int {
        kRateNormal = 0,
        kRateSlow = 1,
        kRateFast = 2,
        kRateCatchUp = 3,
    };

    // Field names are not in the reference map. Latencies are in frames.
    int mDriver;
    int mIndex;
    Symbol mDriverName;
    float mFrequency;  // Playback frequency last set on the channel.
    CritSec mCritSec;
    FMOD::Sound* mSound;  // Looping record buffer.
    FMOD::Channel* mChannel;  // Plays the record buffer back.
    unsigned int mSoundLengthBytes;
    unsigned int mSoundLength;
    unsigned int mLastRecordPos;
    unsigned int mRecordedFrames;
    unsigned int mMinLatency;
    unsigned int mTargetLatency;
    int mMaxLatency;
    unsigned int mLatencyTolerance;
    unsigned int mMinRecordAdvance;
    float mSmoothedLatency;
    RateState mRateState;
    int mLastReadPos;  // Record position _Poll has read up to; -1 at first.
    FMOD::Studio::EventInstance* mEventInstance;
    FMOD::ChannelGroup* mPlaybackGroup;
};

static_assert(offsetof(Mic_FMOD, mDriver) == 16600);
static_assert(offsetof(Mic_FMOD, mDriverName) == 16608);
static_assert(offsetof(Mic_FMOD, mFrequency) == 16616);
static_assert(offsetof(Mic_FMOD, mCritSec) == 16624);
static_assert(offsetof(Mic_FMOD, mSound) == 16640);
static_assert(offsetof(Mic_FMOD, mChannel) == 16648);
static_assert(offsetof(Mic_FMOD, mSoundLengthBytes) == 16656);
static_assert(offsetof(Mic_FMOD, mLastRecordPos) == 16664);
static_assert(offsetof(Mic_FMOD, mMaxLatency) == 16680);
static_assert(offsetof(Mic_FMOD, mSmoothedLatency) == 16692);
static_assert(offsetof(Mic_FMOD, mLastReadPos) == 16700);
static_assert(offsetof(Mic_FMOD, mEventInstance) == 16704);
static_assert(offsetof(Mic_FMOD, mPlaybackGroup) == 16712);
static_assert(sizeof(Mic_FMOD) == 16720);
