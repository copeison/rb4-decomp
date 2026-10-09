#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

struct PlayArgs;

// Base of every microphone. The vtable is at 0x18E63E0; the destructor is at
// 0xE1530. The mic module has not been reconstructed, so only the members the
// FMOD microphone uses are declared.
class Mic {
public:
    // Inlined into Mic_FMOD's constructor at 0x27B3E0, which calls the copy
    // at 0x27B500.
    Mic();
    virtual ~Mic();                       // slots 0-1: 0xE1530, 0xE1590
    // Slot 2 at 0xE1A00: 0 while free, 2 once bound to hardware. Name not in
    // the reference map.
    virtual int GetStatus() const;
    virtual int GetType() const;          // slot 3: 0xE1A10
    virtual bool IsRunning() const = 0;   // slot 4
    virtual void Poll();                  // slot 5: 0xE1A20. Inferred from the map.
    virtual int GetDroppedSamples();      // slot 6: 0xE1A30. Inferred from the map.
    virtual void MicThreadPoll();         // slot 7: 0xE1A40
    virtual float GetSensitivity() const;  // slot 8: 0xE1A50. Inferred from the map.
    virtual Symbol GetName() const;       // slot 9: 0xE1A60
    virtual void Start() = 0;             // slot 10
    // Slot 11 at 0xE1B00. The map has StartPlayback().
    virtual void StartPlayback(const PlayArgs& args);
    virtual void Stop() = 0;              // slot 12
    // Slots 13-14 push the stored volume and mute to the device. Names not
    // in the reference map.
    virtual void _ApplyVolume() = 0;
    virtual void _ApplyMute() = 0;
    virtual void StopPlayback() = 0;      // slot 15

    // Releases a disconnected device. At 0xE16B0. The map names
    // Mic_FMOD::_HandleMicRemoval().
    void _HandleMicRemoval();

    // Field names are not in the reference map.
    int mSampleRate;
    unsigned char mUnknown12[16440];
    float mVolume;
    bool mMuted;
    unsigned char mUnknown16457[143];
};

static_assert(offsetof(Mic, mSampleRate) == 8);
static_assert(offsetof(Mic, mVolume) == 16452);
static_assert(offsetof(Mic, mMuted) == 16456);
static_assert(sizeof(Mic) == 16600);
