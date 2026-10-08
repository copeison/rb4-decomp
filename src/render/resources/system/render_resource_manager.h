#pragma once

#include <cstddef>
#include <cstdint>

#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

struct RenderSystem;

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
    void* scene_block;
    void* time;
    void* smoothness_decay;
    void* sgraph_trans_infos;
    void* scene_global_floats;
    void* scene_global_colors;
    void* tiled_lighting_params;
    void* fog_params;
    void* volumetric_params_0;
    void* volumetric_params_1;
    void* render_target_block;
    void* target_dimensions;
    void* camera_block;
    void* camera_near_far_params;
    void* camera_misc_params;
    void* camera_view_extents;
    void* reserved_216;
    void* reserved_224;
    void* camera_rt_sliced_data;
    void* clip_planes_block;
    void* clip_planes;
    void* skeleton_block;
    void* skeleton_bone_transforms;
    void* misc_draw_state_block;
    void* environment_index;
    void* solid_color;
    void* occlusion_query_block;
    void* occlusion_query_coverage;
    void* debug_block;
    void* debug_modes;
    void* debug_color;
    void* batch_info;
    void* preview_node_index;
    void* transient_blocks[3];
    void* constant_registry;
};

struct RenderResourceManagerRuntime {
    RenderShaderParameterRegistrySet* shader_parameters;
    void* function_table_texture;
    void* resources[35];
    std::uint64_t reserved_680;
};

struct RenderResourceManager {
    RenderShaderParameterBinding shader_parameter_bindings[4];
    RenderShaderConstantState shader_constants;
    RenderResourceManagerRuntime runtime;
    RenderResourcePointerArray* pointer_array;
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
static_assert(offsetof(RenderResourceManager, pointer_array) == 688);
static_assert(offsetof(RenderResourceManager, primary_list) == 696);
static_assert(offsetof(RenderResourceManager, secondary_list) == 704);
static_assert(sizeof(RenderResourceManager) == 712);

RenderResourceManager& render_system_resource_manager(RenderSystem& system);
void render_resource_manager_construct(RenderResourceManager& manager);
void render_resource_manager_initialize_shader_parameters(
    RenderResourceManager& manager);
void render_resource_manager_destruct(RenderResourceManager& manager);
void render_resource_manager_finalize(RenderResourceManager& manager);
void render_resource_manager_shutdown(RenderResourceManager& manager);
void render_resource_manager_reload_shaders(RenderResourceManager& manager);

}  // namespace rb4
