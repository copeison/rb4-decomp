#pragma once

namespace Hmx {

class Timer {
public:
    // Inlined as rdtsc. Name not in the reference map.
    static unsigned long GetCycleCounter() {
        return __builtin_ia32_rdtsc();
    }

    // At 0x25C0E0.
    static double CyclesToMs(unsigned long cycles);
};

}  // namespace Hmx
