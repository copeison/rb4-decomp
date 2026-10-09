#pragma once

namespace Hmx {

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
};

}  // namespace Hmx
