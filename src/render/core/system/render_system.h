#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner_list.h"

namespace rb4 {

struct RenderSystem;

struct RenderSystemVtable {
    void* reserved_0[6];
    void (*prepare_frame)(RenderSystem* system, bool auxiliary_frame);
    void (*submit_frame)(
        RenderSystem* system,
        RenderFrameOwnerList* frame_owners,
        bool auxiliary_frame);
    void* reserved_64[5];
    std::uint32_t (*frame_phase)(RenderSystem* system);
};

struct RenderSystem {
    const RenderSystemVtable* virtual_table;
};

static_assert(offsetof(RenderSystemVtable, prepare_frame) == 48);
static_assert(offsetof(RenderSystemVtable, submit_frame) == 56);
static_assert(offsetof(RenderSystemVtable, frame_phase) == 104);
static_assert(sizeof(RenderSystem) == 8);

void render_system_platform_prepare_frame(
    RenderSystem& system,
    bool auxiliary_frame);
void render_system_platform_submit_frame(
    RenderSystem& system,
    RenderFrameOwnerList& frame_owners,
    bool auxiliary_frame);
std::uint32_t render_system_frame_phase(RenderSystem& system);

}  // namespace rb4
