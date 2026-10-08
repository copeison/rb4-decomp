#pragma once

namespace rb4 {

struct RenderSystem;
struct RenderTargetState;

void render_system_poll();
bool render_system_is_alive();
bool render_system_begin_frame();
void render_system_end_frame();
void render_system_skip_frame();
void render_system_begin_auxiliary_frame(
    RenderSystem& system,
    RenderTargetState* target_state);
void render_system_finish_auxiliary_frame(RenderSystem& system);

}  // namespace rb4
