#pragma once

namespace rb4 {

using RenderDebugCommandHandler = void (*)();

void register_debug_command(
    const char* name,
    RenderDebugCommandHandler handler);

void render_command_toggle_overlay();
void render_command_overlay_help();
void render_command_set_resolution();
void render_command_set_quality_level();
void render_command_set_drawn_scene_range();
void render_command_set_shading_mode();
void render_command_set_buffer_inspection_mode();

}  // namespace rb4
