#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct GameSystemInitOptions {
    bool option0;
    bool initialize_rendering;
    bool option2;
    std::uint8_t reserved[5];
    std::uint64_t value8;
};

static_assert(offsetof(GameSystemInitOptions, initialize_rendering) == 1);
static_assert(offsetof(GameSystemInitOptions, value8) == 8);
static_assert(sizeof(GameSystemInitOptions) == 16);

}  // namespace rb4
