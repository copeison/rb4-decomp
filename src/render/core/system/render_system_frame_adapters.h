#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderFrameOwner;
struct RenderTarget;
struct RenderTargetStateHandle;
struct RenderSystem;

void render_frame_owner_poll(RenderFrameOwner& owner);
void render_frame_owner_delete(RenderFrameOwner& owner);
void render_frame_owner_begin(RenderFrameOwner& owner);
RenderTargetStateHandle render_frame_owner_target_states(
    const RenderFrameOwner& owner);
void render_system_prepare_frame(RenderSystem& system, bool auxiliary_frame);
void render_system_finish_frame(RenderSystem& system, bool auxiliary_frame);

}  // namespace rb4
