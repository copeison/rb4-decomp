#pragma once

#include <_pthread.h>

#include "os/threading/CritSec.h"

// Condition variable bound to a CritSec by Init. The map names
// Condition::Init(CritSec&); the other members are inlined, so their names are
// not in the reference map.
class Condition {
public:
    Condition() : mReserved(nullptr), mMutex(nullptr) {}
    ~Condition() {
        Destroy();
    }

    void Init(CritSec& critSec) {
        mMutex = &critSec.mCritSec;
        ScePthreadCondattr attributes;
        scePthreadCondattrInit(&attributes);
        scePthreadCondInit(&mCond, &attributes, "Condition");
    }
    // Releases the condition variable; it may be bound again by Init.
    void Destroy() {
        if (mMutex != nullptr) {
            scePthreadCondDestroy(&mCond);
            mMutex = nullptr;
        }
    }
    // The caller holds the bound CritSec.
    void Wait() {
        scePthreadCondWait(&mCond, mMutex);
    }
    void Signal() {
        scePthreadCondSignal(&mCond);
    }

    // Cleared by the constructor (for example at 0x251273 for the poll
    // manager's conditions); nothing reads it. Name not in the reference
    // map.
    void* mReserved;
    ScePthreadMutex* mMutex;
    ScePthreadCond mCond;
};

static_assert(sizeof(Condition) == 24);
