#pragma once

#include <cstdint>

namespace rb4 {

// Minimal-standard Park-Miller generator state. Only the leading seed is
// accessed by the recovered callers.
struct RandomGenerator {
    std::int32_t seed;
};

std::int32_t random_generator_next(RandomGenerator& generator);

}  // namespace rb4
