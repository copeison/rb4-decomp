#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

class FxSend;

// One of the game's four microphone channels (vtable 0x195FF58, 24 slots,
// 0x81C8 bytes): it plays a microphone through an FMOD channel, keeps its
// recent and continuous samples and counts the samples it dropped. Its
// slots follow the map's Mic and Mic_FMOD members (SetVolume, SetMute,
// StartPlayback, StopPlayback, GetRecentBuf...), which this build keeps
// beside the engine's own microphones (mic/core/Mic.h). Names not in the
// reference map unless the map's Mic has them; the evidence for the slot
// names is weak where noted. Declared only.
class MicChannel {
public:
    virtual ~MicChannel();  // slots 0-1: 0x8EDE70, 0x8EDF40
    // Slots 2-4: creates the FMOD sound and plays it; releases it.
    virtual void Start();           // slot 2: 0x8EDF90
    virtual void Stop();            // slot 3: 0x8EE310
    virtual bool IsRunning() const;  // slot 4: 0x8EE3A0
    // Slot 5: 0 without a device, 2 once bound to one, as Mic::GetStatus.
    virtual int GetStatus() const;  // slot 5: 0x8EED30
    // Slot 6: the device's pad, -1 without one.
    virtual int GetPad() const;     // slot 6: 0x8EED40
    virtual void SetMute(bool mute);  // slot 7: 0x8EE440
    virtual void SetSensitivity(float sensitivity);  // slot 8: 0x8EED50
    virtual float GetSensitivity() const;            // slot 9: 0x8EED60
    // Slot 10: the channel's volume in decibels.
    virtual void SetVolume(float volume);  // slot 10: 0x8EE3E0
    // Slots 11-12: empty and false. The names are the map's Mic members
    // that fit; the evidence is weak.
    virtual void SetFxSend(FxSend* send);    // slot 11: 0x8EED70
    virtual bool IsMultiMicDevice() const;  // slot 12: 0x8EED80
    // Slots 13-14: unmute and mute the channel's playback.
    virtual void StartPlayback();  // slot 13: 0x8EED90
    virtual void StopPlayback();   // slot 14: 0x8EEDA0
    // Slot 15: run by the mic reader thread. The name is weak.
    virtual void MicThreadPoll();  // slot 15: 0x8EE480
    // Slots 16-17: the latest samples and their count.
    virtual short* GetRecentBuf(int& count);      // slot 16: 0x8EE3B0
    virtual short* GetContinuousBuf(int& count);  // slot 17: 0x8EE3C0
    // Slot 18: the samples dropped since the last call.
    virtual int GetDroppedSamples();  // slot 18: 0x8EF990
    virtual int GetSampleRate() const;  // slot 19: 0x8EE3D0
    // Slot 20: 0 in this build.
    virtual float GetClipping() const;  // slot 20: 0x8EEDB0
    virtual void MarkAsInUse(bool inUse);  // slot 21: 0x8EEDC0
    virtual bool IsInUse() const;          // slot 22: 0x8EEDD0
    // Slot 23: the device's name.
    virtual const Symbol& GetName() const;  // slot 23: 0x8EEDE0

    // Not modelled.
    unsigned char mOpaque8[12];
    // The gain the pitch detection scales the energy with. The name is
    // weak.
    float mGain;
};

static_assert(offsetof(MicChannel, mGain) == 0x14);

// The game's microphone channels (0x20740 bytes, vtable 0x195FEE0, 13
// slots): four MicChannels and the mic reader thread, created at startup
// (0x8EC9C0, together with theMicClientMapper) and updated by the frame loop
// (0x8ECAC0). Its slots follow the map's MicHwManager_FMOD members. Names
// not in the reference map. Declared only.
class MicChannelMgr {
public:
    virtual ~MicChannelMgr();  // slots 0-1: 0x8ECBC0, 0x8ECC70
    virtual bool IsMicConnected(int index);  // slot 2: 0x8ECD70
    // A forced release skips the index check.
    virtual bool ReleaseMic(int index, bool force);  // slot 3: 0x8ECDA0
    virtual void ReleaseAllMics();                   // slot 4: 0x8ECDF0
    virtual int GetNextAvailableMicID();             // slot 5: 0x8ECE60
    virtual bool CaptureMic(int index, bool force);  // slot 6: 0x8ECF30
    // The connected channels.
    virtual int GetNumMics();               // slot 7: 0x8ED400
    virtual MicChannel* GetMic(int index);  // slot 8: 0x8ECFA0
    virtual bool _Init();                   // slot 9: 0x8ECC90
    virtual bool _DidMicsChange();          // slot 10: 0x8ED4A0
    virtual void _ResetMicsChanged();       // slot 11: 0x8ED4B0
    virtual void _Poll();                   // slot 12: 0x8ECFB0
};

// The channels. At 0x1ADF350. Name not in the reference map.
extern MicChannelMgr* TheMicChannelMgr;
