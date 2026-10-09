#pragma once

#include <_pthread.h>

// Engine-wide audio format. The values are plain globals in the executable;
// the accessors at 0xD3BA0 through 0xD3C20 read and write them.
class Audio {
public:
    // Reconstructed from eboot.elf at 0xD3BA0.
    static double GetSamplesPerSecond();
    // Reconstructed from eboot.elf at 0xD3BB0.
    static double GetSecondsPerSample();
    // Reconstructed from eboot.elf at 0xD3BC0.
    static void SetAudioSystemProperties(double samplesPerSecond, int bufferSize);

    // Names not in the reference map.
    static double sSamplesPerSecond;  // 0x19B02B0
    static double sSecondsPerSample;  // 0x19C9940
    static int sBufferSize;           // 0x19B02A4
    static float sBuffersPerSecond;   // 0x19B02A8
    static double sMsPerBuffer;       // 0x19C9938
};

// Recursive engine mutex. The map emits its out-of-line members in the Audio
// object; the owning header has not been reconstructed yet.
class CritSec {
public:
    CritSec(unsigned long spinCount = 0);
    ~CritSec();

    void Enter() {
        scePthreadMutexLock(&mCritSec);
        ++mEntryCount;
    }
    void Exit() {
        --mEntryCount;
        scePthreadMutexUnlock(&mCritSec);
    }

    int mEntryCount;  // Name not in the reference map.
    ScePthreadMutex mCritSec;
};

static_assert(sizeof(CritSec) == 16);

// Scope helper for CritSec. Name not in the reference map.
class CritSecTracker {
public:
    explicit CritSecTracker(CritSec* critSec) : mCritSec(critSec) {
        mCritSec->Enter();
    }
    ~CritSecTracker() {
        mCritSec->Exit();
    }

private:
    CritSec* mCritSec;
};
