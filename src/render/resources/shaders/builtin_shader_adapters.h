#pragma once

namespace rb4 {

void render_error_shader_install_dispatch(void* shader);
void render_basic_shader_install_dispatch(void* shader);
void render_bink_convert_shader_install_dispatch(void* shader);
void render_bloom_shader_install_dispatch(void* shader);
void render_blur_shader_install_dispatch(void* shader);
void render_display_shading_mode_shader_install_dispatch(void* shader);
void render_display_sphere_map_shader_install_dispatch(void* shader);
void render_display_texture_cube_shader_install_dispatch(void* shader);
void render_linearize_depth_shader_install_dispatch(void* shader);
void render_output_conversion_shader_install_dispatch(void* shader);
void render_refine_scene_mask_shader_install_dispatch(void* shader);
void render_stencil_scene_mask_shader_install_dispatch(void* shader);
void render_test_pattern_shader_install_dispatch(void* shader);
void render_test_shader_install_dispatch(void* shader);

}  // namespace rb4
