#include "utl/time/Timer.h"

#include <kernel.h>

namespace {

// Milliseconds per cycle-counter tick, set by Init. Name not in the
// reference map.
double gMsPerCycle;

}  // namespace

namespace Hmx {

// Reconstructed from eboot.elf at 0x25C0A0.
void Timer::Init() {
    gMsPerCycle = 1000.0 / static_cast<double>(sceKernelGetTscFrequency());
}

// Reconstructed from eboot.elf at 0x25C0E0.
double Timer::CyclesToMs(unsigned long cycles) {
    return static_cast<double>(cycles) * gMsPerCycle;
}

// Reconstructed from eboot.elf at 0x25C110.
unsigned long Timer::MsToCycles(double ms) {
    if (gMsPerCycle == 0.0) {
        return 0;
    }
    return static_cast<unsigned long>(ms / gMsPerCycle);
}

}  // namespace Hmx
