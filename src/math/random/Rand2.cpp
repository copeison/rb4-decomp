#include "math/random/Rand2.h"

// Reconstructed from eboot.elf at 0x117B0E0. Schrage's decomposition of
// seed * 16807 mod (2^31 - 1).
int Rand2::Int() {
    constexpr int kMultiplier = 16807;
    constexpr int kQuotient = 127773;
    constexpr int kRemainder = 2836;
    auto next = kMultiplier * (mSeed % kQuotient) -
        kRemainder * (mSeed / kQuotient);
    if (next <= 0) {
        next += 0x7FFFFFFF;
    }
    mSeed = next;
    return next;
}
