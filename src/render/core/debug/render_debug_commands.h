#pragma once

namespace rb4 {

void render_command_toggle_vsync();
void render_command_toggle_scene_mask();
void render_command_toggle_shadows();
void render_command_toggle_postproc();
void render_command_toggle_tonemapping();
void render_command_toggle_vscat();
void render_command_toggle_multithreaded_rendering();
void render_command_toggle_async_compute();
void render_command_toggle_async_copy();
void render_command_toggle_tiled_light_interpolation();
void render_command_toggle_partial_framerate();
void render_command_toggle_stereo_optimizations();
void render_command_toggle_64_bit_light_accum();
void render_command_toggle_hdr();
void render_command_take_screenshot();
void render_command_cycle_screenshot_resolution();

void render_register_debug_commands();

}  // namespace rb4
