#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/targets/render_target.h"

namespace rb4 {

struct RenderFrameOwner;

struct RenderFrameOwnerVtable {
    void* reserved_0;
    void (*delete_owner)(RenderFrameOwner* owner);
    void* reserved_16;
    RenderTargetStateHandle (*target_states)(
        const RenderFrameOwner* owner);
    void (*poll)(RenderFrameOwner* owner);
    void (*begin)(RenderFrameOwner* owner);
};

struct RenderFrameOwner {
    RenderFrameOwnerVtable* virtual_table;
};

static_assert(sizeof(RenderFrameOwner) == 8);

struct RenderExtent {
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    bool empty() const {
        return width == 0 || height == 0;
    }
};

RenderExtent render_frame_owner_output_extent(const RenderFrameOwner& owner);
void render_frame_owner_delete(RenderFrameOwner& owner);
void render_frame_owner_poll(RenderFrameOwner& owner);
void render_frame_owner_begin(RenderFrameOwner& owner);
RenderTargetStateHandle render_frame_owner_target_states(
    const RenderFrameOwner& owner);
std::uint32_t render_frame_owner_draw_mode(const RenderFrameOwner& owner);
std::uint32_t render_frame_owner_debug_view(const RenderFrameOwner& owner);
void render_frame_owner_set_draw_mode(
    RenderFrameOwner& owner,
    std::uint32_t mode);
void render_frame_owner_set_debug_view(
    RenderFrameOwner& owner,
    std::uint32_t view);

}  // namespace rb4
