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

struct RenderTarget {
    void* implementation;
    std::int32_t attachment_index;
    bool owns_state;
    std::uint8_t reserved_alignment[3];
    RenderTargetState* owned_state;
    RenderTargetState* active_state;
};

struct RenderTargetStateHandle {
    RenderTargetState** states;
    std::size_t count;
};

static_assert(sizeof(RenderTargetState) == 32);
static_assert(sizeof(RenderTarget) == 32);
static_assert(sizeof(RenderTargetStateHandle) == 16);

void render_target_construct(
    RenderTarget& target,
    std::uint32_t state_flags,
    bool create_state);
void render_target_destruct(RenderTarget& target);
void render_target_delete(RenderTarget& target);
std::size_t render_target_active_buffer_index(const RenderTarget& target);
RenderTargetStateHandle render_target_active_state_handle(
    RenderTarget& target);
void render_target_set_state(RenderTarget& target, RenderTargetState* state);

}  // namespace rb4
