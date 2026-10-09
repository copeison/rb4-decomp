#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderTargetState {
    void* implementation;
    std::uint32_t state_flags;
    std::uint32_t reserved_12;
    std::uint32_t draw_mode;
    std::uint32_t debug_view;
    std::uint32_t width;
    std::uint32_t height;
};

struct RenderTargetStateHandle {
    RenderTargetState** states;
    std::size_t count;
};

static_assert(sizeof(RenderTargetState) == 32);
static_assert(sizeof(RenderTargetStateHandle) == 16);

}  // namespace rb4
