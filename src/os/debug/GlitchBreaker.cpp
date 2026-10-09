#include "os/debug/GlitchBreaker.h"

#include <cerrno>
#include <ctime>

#include "os/system/System.h"
#include "utl/text/MakeString.h"

// Reconstructed from eboot.elf at 0x399EA0.
GlitchBreaker::GlitchBreaker(int ms, bool) : mMs(ms), mTerminate(false) {
    mSemaphore.Create(0, 0);
}

// Reconstructed from eboot.elf at 0x399EC0.
GlitchBreaker::~GlitchBreaker() {}

// Reconstructed from eboot.elf at 0x399EF0.
int GlitchBreaker::ThreadMain(void* breaker) {
    static_cast<GlitchBreaker*>(breaker)->_ThreadMain();
    return 0;
}

// Reconstructed from eboot.elf at 0x399F00.
int GlitchBreaker::_ThreadMain() {
    while (!mTerminate) {
        bool reset = false;
        if (mMs != 0) {
            timespec deadline;
            clock_gettime(CLOCK_REALTIME, &deadline);
            deadline.tv_sec += mMs / 1000;
            const long nanoseconds = static_cast<long>(mMs % 1000) * 1000000 + deadline.tv_nsec;
            deadline.tv_sec += nanoseconds / 1000000000;
            deadline.tv_nsec = nanoseconds % 1000000000;
            for (;;) {
                if (sem_timedwait(&mSemaphore.mSem, &deadline) == 0) {
                    reset = true;
                    break;
                }
                if (errno == ETIMEDOUT) {
                    break;
                }
            }
        } else {
            for (;;) {
                if (sem_trywait(&mSemaphore.mSem) == 0) {
                    reset = true;
                    break;
                }
                if (errno == EAGAIN) {
                    break;
                }
            }
        }
        if (!reset) {
            // The frame took too long.
            PlatformDebugBreak();
            mSemaphore.Wait();
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x39A050.
void GlitchBreaker::Terminate() {
    mTerminate = true;
    mSemaphore.Release();
}

// Reconstructed from eboot.elf at 0x39A060.
void GlitchBreaker::Reset() {
    mSemaphore.Release();
}

// Reconstructed from eboot.elf at 0x39A070.
ScopedGlitchBreaker::ScopedGlitchBreaker(int ms) : mBreaker(ms, true) {
    mThread.Init("Unknown Thread!");
    FormatString name("ScopedGlitchBreaker (%d ms)");
    name << ms;
    mThread.Create(GlitchBreaker::ThreadMain, &mBreaker, name.Str(), 0, Thread::kPriorityDefault, 0, 0);
    mThread.mThread.Start();
}

// Reconstructed from eboot.elf at 0x39A1B0.
ScopedGlitchBreaker::~ScopedGlitchBreaker() {
    mBreaker.Terminate();
    mThread.mThread._Join();
    // The binary destroys the breaker's semaphore, inlining the member's
    // destructor, before it kills the thread.
    mThread.mThread._ForceKillThread();
}
