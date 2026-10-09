#include "audio/core/system/Audio.h"

// Initial values of the .data globals. The executable starts at 48 kHz with
// 1024-sample buffers. Names not in the reference map.
double Audio::sSamplesPerSecond = 48000.0;
double Audio::sSecondsPerSample;
int Audio::sBufferSize = 1024;
float Audio::sBuffersPerSecond = 46.875F;
double Audio::sMsPerBuffer;

// Reconstructed from eboot.elf at 0xD3BA0.
double Audio::GetSamplesPerSecond() {
    return sSamplesPerSecond;
}

// Reconstructed from eboot.elf at 0xD3BB0.
double Audio::GetSecondsPerSample() {
    return sSecondsPerSample;
}

// Reconstructed from eboot.elf at 0xD3BC0.
void Audio::SetAudioSystemProperties(double samplesPerSecond, int bufferSize) {
    sSamplesPerSecond = samplesPerSecond;
    sSecondsPerSample = 1.0 / samplesPerSecond;
    sBufferSize = bufferSize;
    sBuffersPerSecond =
        static_cast<float>(samplesPerSecond) / static_cast<float>(bufferSize);
    sMsPerBuffer = 1000.0 / static_cast<double>(sBuffersPerSecond);
}

// Inlined into every static CritSec initializer, for example at 0x275330.
CritSec::CritSec(unsigned long) : mEntryCount(0) {
    ScePthreadMutexattr attributes;
    scePthreadMutexattrInit(&attributes);
    scePthreadMutexattrSettype(&attributes, SCE_PTHREAD_MUTEX_RECURSIVE);
    scePthreadMutexInit(&mCritSec, &attributes, "hx crit sec");
    scePthreadMutexattrDestroy(&attributes);
}

// Reconstructed from the inlined tail of eboot.elf 0xE780. Any entries still
// held by the destroying thread are released before the mutex is destroyed.
CritSec::~CritSec() {
    scePthreadMutexLock(&mCritSec);
    int held = mEntryCount;
    scePthreadMutexUnlock(&mCritSec);
    while (held > 0) {
        held = mEntryCount;
        mEntryCount = held - 1;
        scePthreadMutexUnlock(&mCritSec);
        if (held <= 1) {
            break;
        }
    }
    scePthreadMutexDestroy(&mCritSec);
}
