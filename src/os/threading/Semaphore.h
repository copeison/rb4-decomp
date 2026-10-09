#pragma once

#include <cerrno>
#include <semaphore.h>

// Counting semaphore. Its members are inline; the map emits copies of
// Create, Wait, Release and Destroy in os/GlitchBreaker.o and of the
// destructor in utl/ThreadCall.o.
class Semaphore {
public:
    Semaphore() {
        *reinterpret_cast<int*>(&mSem) = 0;
    }
    ~Semaphore() {
        Destroy();
    }

    // The maximum is not used on this platform.
    void Create(int count, int max) {
        static_cast<void>(max);
        sem_init(&mSem, 0, count);
    }
    void Destroy() {
        if (*reinterpret_cast<int*>(&mSem) != 0) {
            sem_destroy(&mSem);
            *reinterpret_cast<int*>(&mSem) = 0;
        }
    }
    // Waits until the count is positive, retrying interrupted waits. The
    // map's signature is Wait(unsigned int), taking a timeout; this build's
    // waits have none.
    void Wait() {
        while (sem_wait(&mSem) != 0) {
            static_cast<void>(errno);
        }
    }
    void Release() {
        sem_post(&mSem);
    }

    // Never read or written. Name not in the reference map.
    int mReserved;
    // Its first word is zero while it is not created. Name not in the
    // reference map.
    sem_t mSem;
};

static_assert(sizeof(Semaphore) == 20);
