#pragma once

#include <cstddef>
#include <cstdint>

#include "render/resources/shaders/shader_cache_validation.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDefines.h"

class RndShaderError;
class RndShaderBasic;
class RndShaderBinkConvert;
class RndShaderBloom;
class RndShaderBlur;
class RndShaderFXAA;
class RndShaderDOFSprite;
class RndShaderDisplayShadingMode;
class RndShaderDisplaySphereMap;
class RndShaderDisplayTextureCube;
class RndShaderDownsample;
class RndShaderLinearizeDepth;
class RndShaderOutputConversion;
class RndShaderRefineSceneMask;
class RndShaderStencilSceneMask;
class RndShaderTestPattern;
class RndCShaderBlurClassify;
class RndCShaderCalcDepthRange;
class RndCShaderClearBuffer;
class RndCShaderCopyBuffer;
class RndCShaderDOFDiscBlur;
class RndCShaderVScatCalcDensityInscattering;
class RndCShaderVScatAccumScattering;
class RndCShaderVScatDeferred;
class RndCShaderSSAOGen;
class RndCShaderCMAAEdgeDetect;
class RndCShaderCMAAEdgePrune;
class RndCShaderCMAAShapeFit;
class RndCShaderCMAAFinalProcess;
class RndCShaderLinearizeDepth;
class RndCShaderSignedDistance;
class RndCShaderSignedDistanceClassify;
class RndShaderRenderTestSimple;
class RndCShaderRenderTestCompute;
struct RndShaderLink;

namespace rb4 {

struct RenderResourcePointerArray {
    void** begin;
    void** end;
    void** capacity;
    void* allocator;
};

struct RenderShaderConstantState {
    std::uint8_t initialization_phases[8];
    RndShaderCBufferConfig* scene_block;
    std::uint64_t time;
    std::uint64_t smoothness_decay;
    std::uint64_t sgraph_trans_infos;
    std::uint64_t scene_global_floats;
    std::uint64_t scene_global_colors;
    std::uint64_t tiled_lighting_params;
    std::uint64_t fog_params;
    std::uint64_t volumetric_params_0;
    std::uint64_t volumetric_params_1;
    RndShaderCBufferConfig* render_target_block;
    std::uint64_t target_dimensions;
    RndShaderCBufferConfig* camera_block;
    std::uint64_t camera_near_far_params;
    std::uint64_t camera_misc_params;
    std::uint64_t camera_view_extents;
    std::uint64_t reserved_216;
    std::uint64_t reserved_224;
    std::uint64_t camera_rt_sliced_data;
    RndShaderCBufferConfig* clip_planes_block;
    std::uint64_t clip_planes;
    RndShaderCBufferConfig* skeleton_block;
    std::uint64_t skeleton_bone_transforms;
    RndShaderCBufferConfig* misc_draw_state_block;
    std::uint64_t environment_index;
    std::uint64_t solid_color;
    RndShaderCBufferConfig* occlusion_query_block;
    std::uint64_t occlusion_query_coverage;
    RndShaderCBufferConfig* debug_block;
    std::uint64_t debug_modes;
    std::uint64_t debug_color;
    std::uint64_t batch_info;
    std::uint64_t preview_node_index;
    RndShaderCBufferConfig* transient_blocks[3];
    RndShaderFixedDefines* constant_registry;
};

struct RenderResourceManagerResources {
    RndShaderError* error_shader;
    RndShaderBasic* basic_shader;
    RndShaderBinkConvert* bink_convert_shader;
    RndShaderBloom* bloom_shader;
    RndShaderBlur* blur_shader;
    RndShaderFXAA* fxaa_shader;
    RndShaderDOFSprite* dof_sprite_shader;
    RndShaderDisplayShadingMode* display_shading_mode_shader;
    RndShaderDisplaySphereMap* display_sphere_map_shader;
    RndShaderDisplayTextureCube* display_texture_cube_shader;
    RndShaderDownsample* downsample_shader;
    RndShaderLinearizeDepth* linearize_depth_shader;
    RndShaderOutputConversion* output_conversion_shader;
    RndShaderRefineSceneMask* refine_scene_mask_shader;
    RndShaderStencilSceneMask* stencil_scene_mask_shader;
    RndShaderTestPattern* test_pattern_shader;
    void* reserved_16;
    RndCShaderBlurClassify* blur_classify_compute_shader;
    RndCShaderCalcDepthRange* calc_depth_range_compute_shader;
    RndCShaderClearBuffer* clear_buffer_compute_shader;
    RndCShaderCopyBuffer* copy_buffer_compute_shader;
    RndCShaderDOFDiscBlur* dof_disc_blur_compute_shader;
    RndCShaderVScatCalcDensityInscattering* vscat_density_compute_shader;
    RndCShaderVScatAccumScattering* vscat_accumulation_compute_shader;
    RndCShaderVScatDeferred* vscat_deferred_compute_shader;
    RndCShaderSSAOGen* ssao_compute_shader;
    RndCShaderCMAAEdgeDetect* cmaa_edge_detect_compute_shader;
    RndCShaderCMAAEdgePrune* cmaa_edge_prune_compute_shader;
    RndCShaderCMAAShapeFit* cmaa_shape_fit_compute_shader;
    RndCShaderCMAAFinalProcess* cmaa_final_process_compute_shader;
    RndCShaderLinearizeDepth* linearize_depth_compute_shader;
    RndCShaderSignedDistance* signed_distance_compute_shader;
    RndCShaderSignedDistanceClassify* signed_distance_classify_compute_shader;
    RndShaderRenderTestSimple* render_test_shader;
    RndCShaderRenderTestCompute* render_test_compute_shader;
};

struct RenderResourceManagerRuntime {
    RndShaderDefinesGroup* shader_parameters;
    void* function_table_texture;
    RenderResourceManagerResources resources;
    // Low 32 bits: FNV-1a hash of the global shader-constant source.
    std::uint64_t constant_source_hash;
};

struct RenderResourceManager {
    RndShaderDefInfo shader_parameter_bindings[4];
    RenderShaderConstantState shader_constants;
    RenderResourceManagerRuntime runtime;
    RenderShaderCacheDefineArray* shader_cache_defines;
    RndShaderLink* primary_list;
    RndShaderLink* secondary_list;
};

static_assert(sizeof(RenderResourcePointerArray) == 32);
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
