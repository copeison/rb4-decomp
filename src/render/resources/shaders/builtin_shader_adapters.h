#pragma once

namespace rb4 {

void render_fxaa_shader_install_dispatch(void* shader);
void render_dof_sprite_shader_install_dispatch(void* shader);
void render_display_shading_mode_shader_install_dispatch(void* shader);
void render_display_sphere_map_shader_install_dispatch(void* shader);
void render_linearize_depth_shader_install_dispatch(void* shader);
void render_refine_scene_mask_shader_install_dispatch(void* shader);
void render_stencil_scene_mask_shader_install_dispatch(void* shader);
void render_test_pattern_shader_install_dispatch(void* shader);

}  // namespace rb4
