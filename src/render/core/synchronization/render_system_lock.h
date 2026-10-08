#pragma once

namespace rb4 {

struct RenderSystem;

void render_system_lock(RenderSystem& system);
void render_system_unlock(RenderSystem& system);
void render_system_enter_locked_call(RenderSystem& system);
void render_system_leave_locked_call(RenderSystem& system);
void render_system_acquire_frame_lock(RenderSystem& system);
void render_system_release_frame_lock(RenderSystem& system);

}  // namespace rb4
