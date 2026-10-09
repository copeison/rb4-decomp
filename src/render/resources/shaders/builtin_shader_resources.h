#pragma once

#include <cstdint>

class RndComputeBuffer;

namespace rb4 {

struct RenderContext;
struct RenderTexture;

void render_error_shader_construct(void* shader);
void render_error_shader_bind(
    void* shader,
    void* context,
    std::int32_t geometry_type);
void render_basic_shader_construct(void* shader);
void render_bink_convert_shader_construct(void* shader);
void render_bloom_shader_construct(void* shader);
void render_blur_shader_construct(void* shader);
void render_fxaa_shader_construct(void* shader);
void render_dof_sprite_shader_construct(void* shader);
void render_display_shading_mode_shader_construct(void* shader);
void render_display_sphere_map_shader_construct(void* shader);
void render_display_texture_cube_shader_construct(void* shader);
void render_downsample_shader_construct(void* shader);
void render_linearize_depth_shader_construct(void* shader);
void render_output_conversion_shader_construct(void* shader);
void render_refine_scene_mask_shader_construct(void* shader);
void render_stencil_scene_mask_shader_construct(void* shader);
void render_test_pattern_shader_construct(void* shader);

void render_blur_classify_compute_shader_construct(void* shader);
void render_calc_depth_range_compute_shader_construct(void* shader);
void render_clear_buffer_compute_shader_construct(void* shader);
void render_copy_buffer_compute_shader_construct(void* shader);
void render_dof_disc_blur_compute_shader_construct(void* shader);
void render_vscat_density_compute_shader_construct(void* shader);
void render_vscat_accumulation_compute_shader_construct(void* shader);
void render_vscat_deferred_compute_shader_construct(void* shader);
void render_ssao_compute_shader_construct(void* shader);
void render_cmaa_edge_detect_compute_shader_construct(void* shader);
void render_cmaa_edge_prune_compute_shader_construct(void* shader);
void render_cmaa_shape_fit_compute_shader_construct(void* shader);
void render_cmaa_final_process_compute_shader_construct(void* shader);
void render_linearize_depth_compute_shader_construct(void* shader);
void render_signed_distance_compute_shader_construct(void* shader);
void render_signed_distance_classify_compute_shader_construct(void* shader);
void render_test_shader_construct(void* shader);
void render_test_compute_shader_construct(void* shader);

void render_linearize_depth_shader_draw(
    void* shader,
    RenderContext& context,
    RenderTexture& depth);
void render_refine_scene_mask_shader_draw(
    void* shader,
    RenderContext& context,
    RenderTexture& unrefined_mask);
void render_display_sphere_map_shader_draw(
    void* shader,
    RenderContext& context,
    RenderTexture& texture);
void render_dof_sprite_shader_draw(
    void* shader,
    RenderContext& context,
    RenderTexture& bokeh,
    RndComputeBuffer& sprites);

}  // namespace rb4
