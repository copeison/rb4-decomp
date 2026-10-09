#pragma once

#include <cstddef>

#include "audio/core/system/Audio.h"
#include "audio/fmod/api/fmod_api.h"
#include "mic/core/Mic.h"

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
    void StopPlayback() override;         // slot 15: 0x27BF80

    // Binds the mic to a record driver whose name still matches. Reconstructed
    // from eboot.elf at 0x27B780.
    bool AttachToHardware(int driver, Symbol name);
    // Reconstructed from eboot.elf at 0x27B880. A missing or disconnected
    // driver detaches the mic.
    bool CheckDeviceStillConnected();

    // Field names are not in the reference map.
    int mDriver;
    int mIndex;
    Symbol mDriverName;
    float mFrequency;
    CritSec mCritSec;
    FMOD::Sound* mSound;
    FMOD::Channel* mChannel;
    unsigned int mSoundLength;
    unsigned char mUnknown16660[20];
    int mUnknown16680;
    int mUnknown16684;
    int mUnknown16688;
    int mUnknown16692;
    int mUnknown16696;
    FMOD::Studio::EventInstance* mEventInstance;
    FMOD::ChannelGroup* mPlaybackGroup;
};

static_assert(offsetof(Mic_FMOD, mDriver) == 16600);
static_assert(offsetof(Mic_FMOD, mDriverName) == 16608);
static_assert(offsetof(Mic_FMOD, mFrequency) == 16616);
static_assert(offsetof(Mic_FMOD, mCritSec) == 16624);
static_assert(offsetof(Mic_FMOD, mSound) == 16640);
static_assert(offsetof(Mic_FMOD, mChannel) == 16648);
static_assert(offsetof(Mic_FMOD, mSoundLength) == 16656);
static_assert(offsetof(Mic_FMOD, mUnknown16680) == 16680);
static_assert(offsetof(Mic_FMOD, mEventInstance) == 16704);
static_assert(offsetof(Mic_FMOD, mPlaybackGroup) == 16712);
static_assert(sizeof(Mic_FMOD) == 16720);
