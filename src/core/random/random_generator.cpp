#include "core/random/random_generator.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x117B0E0. Schrage's decomposition of
// seed * 16807 mod (2^31 - 1).
std::int32_t random_generator_next(RandomGenerator& generator) {
    constexpr std::int32_t kMultiplier = 16807;
    constexpr std::int32_t kQuotient = 127773;
    constexpr std::int32_t kRemainder = 2836;
    const auto seed = generator.seed;
    auto next = kMultiplier * (seed % kQuotient) -
        kRemainder * (seed / kQuotient);
    if (next <= 0) {
        next += 0x7FFFFFFF;
    }
    generator.seed = next;
    return next;
}

}  // namespace rb4
