#pragma once

#include <cstddef>
#include <cstdint>

#include "render/resources/shaders/shader_cache_validation.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

struct RenderResourcePointerArray {
    void** begin;
    void** end;
    void** capacity;
    void* allocator;
};

struct RenderResourceListNode {
    RenderResourceListNode* next;
    RenderResourceListNode* previous;
};

struct RenderShaderConstantState {
    std::uint8_t initialization_phases[8];
    RenderShaderConstantBlock* scene_block;
    std::uint64_t time;
    std::uint64_t smoothness_decay;
    std::uint64_t sgraph_trans_infos;
    std::uint64_t scene_global_floats;
    std::uint64_t scene_global_colors;
    std::uint64_t tiled_lighting_params;
    std::uint64_t fog_params;
    std::uint64_t volumetric_params_0;
    std::uint64_t volumetric_params_1;
    RenderShaderConstantBlock* render_target_block;
    std::uint64_t target_dimensions;
    RenderShaderConstantBlock* camera_block;
    std::uint64_t camera_near_far_params;
    std::uint64_t camera_misc_params;
    std::uint64_t camera_view_extents;
    std::uint64_t reserved_216;
    std::uint64_t reserved_224;
    std::uint64_t camera_rt_sliced_data;
    RenderShaderConstantBlock* clip_planes_block;
    std::uint64_t clip_planes;
    RenderShaderConstantBlock* skeleton_block;
    std::uint64_t skeleton_bone_transforms;
    RenderShaderConstantBlock* misc_draw_state_block;
    std::uint64_t environment_index;
    std::uint64_t solid_color;
    RenderShaderConstantBlock* occlusion_query_block;
    std::uint64_t occlusion_query_coverage;
    RenderShaderConstantBlock* debug_block;
    std::uint64_t debug_modes;
    std::uint64_t debug_color;
    std::uint64_t batch_info;
    std::uint64_t preview_node_index;
    RenderShaderConstantBlock* transient_blocks[3];
    RenderShaderConstantRegistry* constant_registry;
};

struct RenderResourceManagerResources {
    void* error_shader;
    void* basic_shader;
    void* bink_convert_shader;
    void* bloom_shader;
    void* blur_shader;
    void* fxaa_shader;
    void* dof_sprite_shader;
    void* display_shading_mode_shader;
    void* display_sphere_map_shader;
    void* display_texture_cube_shader;
    void* downsample_shader;
    void* linearize_depth_shader;
    void* output_conversion_shader;
    void* refine_scene_mask_shader;
    void* stencil_scene_mask_shader;
    void* test_pattern_shader;
    void* reserved_16;
    void* blur_classify_compute_shader;
    void* calc_depth_range_compute_shader;
    void* clear_buffer_compute_shader;
    void* copy_buffer_compute_shader;
    void* dof_disc_blur_compute_shader;
    void* vscat_density_compute_shader;
    void* vscat_accumulation_compute_shader;
    void* vscat_deferred_compute_shader;
    void* ssao_compute_shader;
    void* cmaa_edge_detect_compute_shader;
    void* cmaa_edge_prune_compute_shader;
    void* cmaa_shape_fit_compute_shader;
    void* cmaa_final_process_compute_shader;
    void* linearize_depth_compute_shader;
    void* signed_distance_compute_shader;
    void* signed_distance_classify_compute_shader;
    void* render_test_shader;
    void* render_test_compute_shader;
};

struct RenderResourceManagerRuntime {
    RenderShaderParameterRegistrySet* shader_parameters;
    void* function_table_texture;
    RenderResourceManagerResources resources;
    // Low 32 bits: FNV-1a hash of the global shader-constant source.
    std::uint64_t constant_source_hash;
};

struct RenderResourceManager {
    RenderShaderParameterBinding shader_parameter_bindings[4];
    RenderShaderConstantState shader_constants;
    RenderResourceManagerRuntime runtime;
    RenderShaderCacheDefineArray* shader_cache_defines;
    RenderResourceListNode* primary_list;
    RenderResourceListNode* secondary_list;
};

static_assert(sizeof(RenderResourcePointerArray) == 32);
static_assert(sizeof(RenderResourceListNode) == 16);
static_assert(sizeof(RenderShaderConstantState) == 304);
static_assert(
    offsetof(RenderResourceManager, shader_parameter_bindings) == 0);
static_assert(offsetof(RenderResourceManager, shader_constants) == 80);
static_assert(
    offsetof(RenderShaderConstantState, scene_block) == 8);
static_assert(
    offsetof(RenderShaderConstantState, render_target_block) == 88);
static_assert(
    offsetof(RenderShaderConstantState, camera_block) == 104);
static_assert(
    offsetof(RenderShaderConstantState, clip_planes_block) == 160);
static_assert(
    offsetof(RenderShaderConstantState, skeleton_block) == 176);
static_assert(
    offsetof(RenderShaderConstantState, misc_draw_state_block) == 192);
static_assert(
    offsetof(RenderShaderConstantState, occlusion_query_block) == 216);
static_assert(
    offsetof(RenderShaderConstantState, debug_block) == 232);
static_assert(
    offsetof(RenderShaderConstantState, transient_blocks) == 272);
static_assert(
    offsetof(RenderShaderConstantState, constant_registry) == 296);
static_assert(sizeof(RenderResourceManagerRuntime) == 304);
static_assert(sizeof(RenderResourceManagerResources) == 280);
static_assert(
    offsetof(RenderResourceManagerResources, dof_sprite_shader) == 48);
static_assert(
    offsetof(RenderResourceManagerResources, reserved_16) == 128);
static_assert(
    offsetof(
        RenderResourceManagerResources,
        blur_classify_compute_shader) == 136);
static_assert(
    offsetof(RenderResourceManagerResources, render_test_shader) == 264);
static_assert(
    offsetof(
        RenderResourceManagerResources,
        render_test_compute_shader) == 272);
static_assert(offsetof(RenderResourceManager, runtime) == 384);
static_assert(
    offsetof(RenderResourceManager, runtime) +
        offsetof(RenderResourceManagerRuntime, function_table_texture) ==
    392);
static_assert(
    offsetof(RenderResourceManager, runtime) +
        offsetof(RenderResourceManagerRuntime, shader_parameters) ==
    384);
static_assert(
    offsetof(RenderResourceManager, runtime) +
        offsetof(RenderResourceManagerRuntime, resources) ==
    400);
static_assert(offsetof(RenderResourceManager, shader_cache_defines) == 688);
static_assert(offsetof(RenderResourceManager, primary_list) == 696);
static_assert(offsetof(RenderResourceManager, secondary_list) == 704);
static_assert(sizeof(RenderResourceManager) == 712);

void render_resource_manager_construct(RenderResourceManager& manager);
void render_resource_manager_initialize(RenderResourceManager& manager);
void render_resource_manager_initialize_shader_parameters(
    RenderResourceManager& manager);
void render_resource_manager_initialize_shader_constant_registry(
    RenderResourceManager& manager);
void render_resource_manager_initialize_shader_constants(
    RenderResourceManager& manager);
void render_resource_manager_destruct(RenderResourceManager& manager);
void render_resource_manager_finalize(RenderResourceManager& manager);
void render_resource_manager_shutdown(RenderResourceManager& manager);
void render_resource_manager_reload_shaders(RenderResourceManager& manager);

}  // namespace rb4
