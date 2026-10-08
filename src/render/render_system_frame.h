#pragma once

namespace rb4 {

void render_system_poll();
bool render_system_is_alive();
bool render_system_begin_frame();
void render_system_end_frame();
void render_system_skip_frame();

}  // namespace rb4
