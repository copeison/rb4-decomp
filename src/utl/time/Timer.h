#pragma once

#include <cstddef>

namespace Hmx {

// A cycle-counter stopwatch. The instance members are inline in the
// binary; only those the reconstructed code reads are declared.
class Timer {
public:
    // Inlined as rdtsc. Name not in the reference map.
    static unsigned long GetCycleCounter() {
        return __builtin_ia32_rdtsc();
    }

    // Measures the cycle counter's frequency.
    static void Init();  // 0x25C0A0
    static double CyclesToMs(unsigned long cycles);  // 0x25C0E0
    // Zero before Init.
    static unsigned long MsToCycles(double ms);  // 0x25C110

    // Adds the cycles since the last split while running and returns the
    // total in milliseconds. Name not in the reference map.
    float SplitMs() {
        if (mRunning > 0) {
            const unsigned long now = GetCycleCounter();
            mCycles += now - mStart;
            mStart = now;
        }
        return static_cast<float>(CyclesToMs(mCycles));
    }

    // Field names are not in the reference map.
    unsigned long mStart;
    unsigned long mCycles;
    int mRunning;  // Positive while running.
};

static_assert(offsetof(Timer, mCycles) == 8);
static_assert(offsetof(Timer, mRunning) == 16);

}  // namespace Hmx
