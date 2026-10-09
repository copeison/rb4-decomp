#pragma once

#include <_pthread.h>

// Recursive engine mutex. The map emits its members as inline copies (for
// example in audio/SoundManager.o), so they are defined in this header and in
// CritSec.cpp.
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

// Scope helpers named in the reference map.
class ScopedCritSec {
public:
    explicit ScopedCritSec(CritSec& critSec) : mCritSec(critSec) {
        mCritSec.Enter();
    }
    ~ScopedCritSec() {
        mCritSec.Exit();
    }

private:
    CritSec& mCritSec;
};

class ScopedCritSecPtr {
public:
    explicit ScopedCritSecPtr(CritSec* critSec) : mCritSec(critSec) {
        mCritSec->Enter();
    }
    ~ScopedCritSecPtr() {
        mCritSec->Exit();
    }

private:
    CritSec* mCritSec;
};
