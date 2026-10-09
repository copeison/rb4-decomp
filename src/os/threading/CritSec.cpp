#include "os/threading/CritSec.h"

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
