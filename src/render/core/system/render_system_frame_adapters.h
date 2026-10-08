#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderFrameOwner;
struct RenderTarget;
struct RenderTargetStateHandle;
struct RenderSystem;

void render_system_lock(RenderSystem& system);
void render_system_unlock(RenderSystem& system);
void render_system_enter_locked_call(RenderSystem& system);
void render_system_leave_locked_call(RenderSystem& system);

void render_frame_owner_poll(RenderFrameOwner& owner);
void render_frame_owner_delete(RenderFrameOwner& owner);
RenderTargetStateHandle render_frame_owner_target_states(
    const RenderFrameOwner& owner);
void render_system_prepare_frame(RenderSystem& system, bool auxiliary_frame);
bool render_system_attach_frame_owner(
    RenderSystem& system,
    RenderFrameOwner& owner);
void render_system_clear_active_frame(RenderSystem& system);
void render_system_finish_frame(RenderSystem& system, bool auxiliary_frame);

}  // namespace rb4
