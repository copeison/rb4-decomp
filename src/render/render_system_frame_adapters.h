#pragma once

namespace rb4 {

struct RenderFrameOwner;
struct RenderSystem;

RenderSystem* render_system_instance();

void render_system_lock(RenderSystem& system);
void render_system_unlock(RenderSystem& system);
void render_system_enter_locked_call(RenderSystem& system);
void render_system_leave_locked_call(RenderSystem& system);

RenderFrameOwner* render_system_frame_owner(RenderSystem& system);
RenderFrameOwner* render_system_active_frame_owner(RenderSystem& system);
void render_frame_owner_poll(RenderFrameOwner& owner);

void render_system_prepare_frame(RenderSystem& system, bool auxiliary_frame);
bool render_system_attach_frame_owner(
    RenderSystem& system,
    RenderFrameOwner& owner);
void render_system_clear_active_frame(RenderSystem& system);
void render_system_finish_frame(RenderSystem& system, bool auxiliary_frame);
void render_system_increment_skipped_frame_count(RenderSystem& system);

}  // namespace rb4
