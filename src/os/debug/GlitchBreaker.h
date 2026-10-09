#pragma once

#include <cstddef>

#include "os/threading/Semaphore.h"
#include "utl/threading/Thread.h"

// A watchdog that breaks into the debugger when it is not reset within its
// limit (os/GlitchBreaker.o). SystemInit starts one for the
// glitch_break_ms option.
class GlitchBreaker {
public:
    // The flag is not used in this build.
    GlitchBreaker(int ms, bool enabled);  // 0x399EA0
    ~GlitchBreaker();  // 0x399EC0
    // The watchdog thread's entry.
    static int ThreadMain(void* breaker);  // 0x399EF0
    // Stops the watchdog loop.
    void Terminate();  // 0x39A050
    // Restarts the watchdog's wait.
    void Reset();  // 0x39A060

private:
    int _ThreadMain();  // 0x399F00

    // Field names are not in the reference map.
    Semaphore mSemaphore;
    // The limit; zero only polls the semaphore.
    int mMs;
    bool mTerminate;
};

static_assert(sizeof(GlitchBreaker) == 28);

// A watchdog on its own thread for the lifetime of the object.
class ScopedGlitchBreaker {
public:
    explicit ScopedGlitchBreaker(int ms);  // 0x39A070
    ~ScopedGlitchBreaker();  // 0x39A1B0

private:
    // Field names are not in the reference map.
    NamedThread mThread;
    GlitchBreaker mBreaker;
};

static_assert(sizeof(ScopedGlitchBreaker) == 0xA8);
