#include "render/resources/system/render_resource_manager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/core/settings/render_settings.h"
#include "render/core/platform/render_platform_config.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture_array_1d.h"
#include "render/core/textures/render_texture_mip_chain.h"
#include "render/resources/names/render_resource_name.h"
#include "render/resources/shaders/builtin_shader_resources.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/primary_shader_resource_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kResourceManagerOffset = 2544;
constexpr std::ptrdiff_t kSecondaryShaderDirtyOffset = -395;
constexpr std::size_t kCurrentPlatformConfigIndex = 7;
constexpr std::uint32_t kAsyncComputeFeature = 0x10;

struct RenderManagedObjectDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* object);
};

struct RenderManagedObject {
    RenderManagedObjectDispatch* dispatch;
};

RenderResourceListNode* create_list_sentinel() {
    auto* node = static_cast<RenderResourceListNode*>(
        render_allocate(sizeof(RenderResourceListNode)));
    node->next = node;
    node->previous = node;
    return node;
}

void release_list_sentinel(RenderResourceListNode*& node) {
    if (node == nullptr) {
        return;
    }

    node->next->previous = node->previous;
    node->previous->next = node->next;
    render_release(node);
    node = nullptr;
}

void release_dynamic_resource(void*& storage) {
    auto* resource = static_cast<RenderManagedObject*>(storage);
    if (resource != nullptr) {
        resource->dispatch->release_dynamic(resource);
        storage = nullptr;
    }
}

void destruct_parameter_registry(RenderShaderParameterRegistry& parameters) {
    for (auto* parameter = parameters.begin;
         parameter != parameters.end;
         ++parameter) {
        render_resource_name_destruct(parameter->name);
    }
    if (parameters.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(parameters.capacity) -
            reinterpret_cast<std::uint8_t*>(parameters.begin));
        engine_deallocate_sized(parameters.begin, byte_count);
    }
}

// Reconstructed from eboot.elf at 0x645F20.
float sample_function_table(
    std::uint32_t function_index,
    float input) {
    float output = 0.0F;
    switch (function_index) {
    case 0:
        output = 1.0F - input;
        break;
    case 1: {
        const auto denominator = 1.25F * input + 0.25F;
        output = 0.0642857179F / (denominator * denominator) -
            0.0285714306F;
        break;
    }
    case 2: {
        constexpr float kMinimum = 0.006737947F;
        const auto exponential = 1.0F / std::exp(5.0F * input);
        output = (exponential - kMinimum) / (1.0F - kMinimum);
        break;
    }
    case 3: {
        constexpr float kMinimum = 0.006692851F;
        constexpr float kMaximum = 0.993307173F;
        const auto sigmoid =
            1.0F / (std::exp((input - 0.5F) * 10.0F) + 1.0F);
        output = (sigmoid - kMinimum) / (kMaximum - kMinimum);
        break;
    }
    default:
        break;
    }
    return std::max(0.0F, std::min(1.0F, output));
}

struct ShaderConstantDefinition {
    const char* name;
    std::int32_t value;
};

template <std::size_t Count>
void add_shader_constant_group(
    RenderShaderConstantRegistry& registry,
    const char* comment,
    const ShaderConstantDefinition (&definitions)[Count]) {
    render_shader_constant_registry_add_comment(registry, comment);
    for (const auto& definition : definitions) {
        render_shader_constant_registry_add_definition(
            registry, definition.name, definition.value);
    }
}

void* create_builtin_shader(
    std::size_t size,
    void (*construct)(void*)) {
    auto* storage = render_allocate(size);
    construct(storage);
    render_primary_shader_register(
        *static_cast<RenderPrimaryShaderResource*>(storage));
    return storage;
}

bool supports_async_compute() {
    const auto& platform = render_system_platform_config_at(
        *render_system_instance(), kCurrentPlatformConfigIndex);
    return (platform.feature_flags & kAsyncComputeFeature) != 0;
}

}  // namespace

RenderResourceManager& render_system_resource_manager(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderResourceManager*>(
        bytes + kResourceManagerOffset);
}

// Reconstructed from eboot.elf at 0x63F180.
void render_resource_manager_construct(RenderResourceManager& manager) {
    for (auto& binding : manager.shader_parameter_bindings) {
        binding = {};
    }

    auto* constant_words = reinterpret_cast<std::int64_t*>(
        &manager.shader_constants);
    std::fill_n(constant_words, 34, std::int64_t{-1});
    for (const auto index : {0U, 1U, 11U, 13U, 20U, 22U, 24U, 27U, 29U}) {
        constant_words[index] = 0;
    }
    std::fill_n(constant_words + 34, 4, std::int64_t{0});
    manager.runtime = {};

    manager.pointer_array = static_cast<RenderResourcePointerArray*>(
        render_allocate(sizeof(RenderResourcePointerArray)));
    *manager.pointer_array = {};
    manager.primary_list = create_list_sentinel();
    manager.secondary_list = create_list_sentinel();
}

// Reconstructed from eboot.elf at 0x640BF0.
void render_resource_manager_initialize_shader_parameters(
    RenderResourceManager& manager) {
    auto* parameters = static_cast<RenderShaderParameterRegistrySet*>(
        render_allocate(sizeof(RenderShaderParameterRegistrySet)));
    render_shader_parameter_registry_set_construct(*parameters);
    manager.runtime.shader_parameters = parameters;

    struct BindingDefinition {
        const char* name;
        std::uint32_t first_value;
        std::uint32_t last_value;
        std::size_t registry_index;
    };
    constexpr BindingDefinition kBindings[] = {
        {"HX_BT709_TO_BT2020", 0, 2, 0},
        {"HX_NUM_RT_SLICES", 0, 7, 0},
        {"HX_SHADING_MODE", 0, 19, 0},
        {"HX_GEO_TYPE", 0, 2, 1},
    };

    for (std::size_t index = 0; index < 4; ++index) {
        const auto& definition = kBindings[index];
        const Symbol name(definition.name);
        render_shader_parameter_registry_add(
            &manager.shader_parameter_bindings[index],
            &parameters->registries[definition.registry_index],
            name.value(),
            definition.first_value,
            definition.last_value);
    }
}

// Reconstructed from eboot.elf at 0x63F920.
void render_resource_manager_initialize_shader_constant_registry(
    RenderResourceManager& manager) {
    auto* registry = static_cast<RenderShaderConstantRegistry*>(
        render_allocate(sizeof(RenderShaderConstantRegistry)));
    render_shader_constant_registry_construct(*registry);
    manager.shader_constants.constant_registry = registry;

    constexpr ShaderConstantDefinition kMiscConstants[] = {
        {"HX_MAX_BONES", 256},
        {"HX_MAX_CLIP_PLANES", 4},
        {"HX_MAX_TEXARRAY_SIZE", 2048},
        {"HX_VIEWPROJ_OFFSET", 0},
        {"HX_CAMXFM_OFFSET", 4},
        {"HX_CAMXFMINV_OFFSET", 7},
        {"HX_CAM_SIZE_PER_RT_SLICE", 10},
    };
    constexpr ShaderConstantDefinition kProgramTypes[] = {
        {"HX_PROGRAM_TYPE_VERTEX", 0},
        {"HX_PROGRAM_TYPE_HULL", 1},
        {"HX_PROGRAM_TYPE_DOMAIN", 2},
        {"HX_PROGRAM_TYPE_GEOMETRY", 3},
        {"HX_PROGRAM_TYPE_PIXEL", 4},
        {"HX_PROGRAM_TYPE_COMPUTE", 5},
    };
    constexpr ShaderConstantDefinition kGeometryTypes[] = {
        {"HX_GEO_UNSKINNED_MESH", 0},
        {"HX_GEO_SKINNED_MESH", 1},
    };
    constexpr ShaderConstantDefinition kStereoEyes[] = {
        {"HX_STEREO_EYE_LEFT", 0},
        {"HX_STEREO_EYE_RIGHT", 1},
    };
    constexpr ShaderConstantDefinition kBillboardTypes[] = {
        {"HX_BILLBOARD_NONE", 0},
        {"HX_BILLBOARD_CAMERA_XYZ", 1},
        {"HX_BILLBOARD_CAMERA_XY", 2},
        {"HX_BILLBOARD_CAMERA_KEEPZ", 3},
    };
    constexpr ShaderConstantDefinition kShadingModes[] = {
        {"HX_SHADING_MODE_STANDARD", 0},
        {"HX_SHADING_MODE_STANDARD_FOG", 1},
        {"HX_SHADING_MODE_STANDARD_VSCAT", 2},
        {"HX_SHADING_MODE_DEPTH_ONLY", 3},
        {"HX_SHADING_MODE_SOLID_COLOR", 4},
        {"HX_SHADING_MODE_DEFERRED_NORMALS_AND_ZFILL", 5},
        {"HX_SHADING_MODE_DEFERRED_UNLIT", 6},
        {"HX_SHADING_MODE_DEFERRED_UNLIT_AND_ZFILL", 7},
        {"HX_SHADING_MODE_DEFERRED_LIT", 8},
        {"HX_SHADING_MODE_DEFERRED_LIT_AND_ZFILL", 9},
        {"HX_SHADING_MODE_DEFERRED_LIT_EMISSIVE", 10},
        {"HX_SHADING_MODE_DEFERRED_LIT_EMISSIVE_AND_ZFILL", 11},
        {"HX_SHADING_MODE_DEFERRED_DECAL_TRANSPARENT", 12},
        {"HX_SHADING_MODE_FWD_LIT_OPAQUE", 13},
        {"HX_SHADING_MODE_SCENE_MASK", 14},
        {"HX_SHADING_MODE_IMPOSTOR_MAPS", 15},
        {"HX_SHADING_MODE_FAST_CHEAP", 16},
        {"HX_SHADING_MODE_WIREFRAME", 17},
        {"HX_SHADING_MODE_DEBUG_MISC", 18},
        {"HX_SHADING_MODE_FIRST_DEBUG", 16},
    };
    constexpr ShaderConstantDefinition kShaderDebugModes[] = {
        {"HX_SHADER_DEBUG_MODE_UNLIT", 2},
        {"HX_SHADER_DEBUG_MODE_OVERDRAW", 3},
        {"HX_SHADER_DEBUG_MODE_BATCHES", 4},
        {"HX_SHADER_DEBUG_MODE_BATCH_SIZE", 5},
        {"HX_SHADER_DEBUG_MODE_LIGHTING_ONLY", 6},
        {"HX_SHADER_DEBUG_MODE_LIT_DIFFUSE", 7},
        {"HX_SHADER_DEBUG_MODE_LIT_SPECULAR", 8},
        {"HX_SHADER_DEBUG_MODE_LIT_DIRECT", 9},
        {"HX_SHADER_DEBUG_MODE_LIT_DIRECT_DIFFUSE", 10},
        {"HX_SHADER_DEBUG_MODE_LIT_DIRECT_SPECULAR", 11},
        {"HX_SHADER_DEBUG_MODE_LIT_INDIRECT", 12},
        {"HX_SHADER_DEBUG_MODE_LIT_INDIRECT_DIFFUSE", 13},
        {"HX_SHADER_DEBUG_MODE_LIT_INDIRECT_SPECULAR", 14},
        {"HX_SHADER_DEBUG_MODE_NO_NEGLIGHTS", 15},
        {"HX_SHADER_DEBUG_MODE_LIGHTING_OVERDRAW", 16},
        {"HX_SHADER_DEBUG_MODE_LIGHT_PROBE_OVERDRAW", 17},
        {"HX_SHADER_DEBUG_MODE_VERTEX_COLOR", 18},
        {"HX_SHADER_DEBUG_MODE_VERTEX_ALPHA", 19},
        {"HX_SHADER_DEBUG_MODE_VERTEX_NORMAL", 20},
        {"HX_SHADER_DEBUG_MODE_VERTEX_TANGENT", 21},
        {"HX_SHADER_DEBUG_MODE_VERTEX_BITANGENT", 22},
        {"HX_SHADER_DEBUG_MODE_PIXEL_NORMAL", 23},
        {"HX_SHADER_DEBUG_MODE_UV0", 24},
        {"HX_SHADER_DEBUG_MODE_UV1", 25},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_LIGHTING_PATH", 26},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_COLOR", 27},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_ALPHA", 28},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_SMOOTHNESS", 29},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_METALLICITY", 30},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_EMISSIVE", 31},
    };
    constexpr ShaderConstantDefinition kCubeFaces[] = {
        {"HX_CUBE_FACE_RIGHT", 0},
        {"HX_CUBE_FACE_LEFT", 1},
        {"HX_CUBE_FACE_TOP", 2},
        {"HX_CUBE_FACE_BOTTOM", 3},
        {"HX_CUBE_FACE_FRONT", 4},
        {"HX_CUBE_FACE_BACK", 5},
    };
    constexpr ShaderConstantDefinition kFrustumPlanes[] = {
        {"HX_FRUSTUM_PLANE_FRONT", 0},
        {"HX_FRUSTUM_PLANE_BACK", 1},
        {"HX_FRUSTUM_PLANE_LEFT", 2},
        {"HX_FRUSTUM_PLANE_RIGHT", 3},
        {"HX_FRUSTUM_PLANE_TOP", 4},
        {"HX_FRUSTUM_PLANE_BOTTOM", 5},
    };
    constexpr ShaderConstantDefinition kFrustumCorners[] = {
        {"HX_FRUSTUM_CORNER_FRONT_LEFT_TOP", 0},
        {"HX_FRUSTUM_CORNER_FRONT_RIGHT_TOP", 1},
        {"HX_FRUSTUM_CORNER_FRONT_LEFT_BOTTOM", 2},
        {"HX_FRUSTUM_CORNER_FRONT_RIGHT_BOTTOM", 3},
        {"HX_FRUSTUM_CORNER_BACK_LEFT_TOP", 4},
        {"HX_FRUSTUM_CORNER_BACK_RIGHT_TOP", 5},
        {"HX_FRUSTUM_CORNER_BACK_LEFT_BOTTOM", 6},
        {"HX_FRUSTUM_CORNER_BACK_RIGHT_BOTTOM", 7},
    };
    constexpr ShaderConstantDefinition kLightTypes[] = {
        {"HX_LIGHT_TYPE_POINT", 0},
        {"HX_LIGHT_TYPE_SPOT", 1},
        {"HX_LIGHT_TYPE_DIRECTIONAL", 2},
        {"HX_NUM_LIGHT_TYPES", 3},
        {"HX_NUM_BOUNDED_LIGHT_TYPES", 2},
        {"HX_ILLUM_POSITIVE", 0},
        {"HX_ILLUM_POSITIVE_DIFFUSE", 1},
        {"HX_ILLUM_POSITIVE_SPECULAR", 2},
        {"HX_ILLUM_NEGATIVE", 3},
        {"HX_LIGHT_COOKIE_STATIC", 0},
        {"HX_LIGHT_COOKIE_RENDERED", 1},
    };
    constexpr ShaderConstantDefinition kLightingPaths[] = {
        {"HX_LIGHTING_PATH_UNLIT", 0},
        {"HX_LIGHTING_PATH_DEFERRED_LIT", 1},
        {"HX_LIGHTING_PATH_DEFERRED_EMISSIVE", 2},
        {"HX_LIGHTING_PATH_FWD_LIT_STANDARD", 3},
        {"HX_LIGHTING_PATH_FWD_LIT_SUBSURFACE", 4},
        {"HX_LIGHTING_PATH_FWD_LIT_SKIN", 5},
        {"HX_LIGHTING_PATH_FWD_LIT_HAIR", 6},
    };
    constexpr ShaderConstantDefinition kCommonStructSizes[] = {
        {"HX_SIZEOF_PLANE", 16},
        {"HX_SIZEOF_SPHERE", 16},
        {"HX_SIZEOF_CSLIGHTIDRANGE", 32},
        {"HX_SIZEOF_CSLIGHTPOINT", 208},
        {"HX_SIZEOF_CSLIGHTSPOT", 352},
        {"HX_SIZEOF_CSLIGHTDIRECTIONAL", 112},
        {"HX_SIZEOF_CSLIGHTPROBE", 96},
    };

    add_shader_constant_group(*registry, "misc constants", kMiscConstants);
    add_shader_constant_group(*registry, "program types", kProgramTypes);
    add_shader_constant_group(*registry, "geometry types", kGeometryTypes);
    add_shader_constant_group(*registry, "stereo eyes", kStereoEyes);
    add_shader_constant_group(
        *registry, "billboarding types", kBillboardTypes);
    add_shader_constant_group(*registry, "shading modes", kShadingModes);
    add_shader_constant_group(
        *registry, "shader debug modes", kShaderDebugModes);
    add_shader_constant_group(*registry, "cube faces", kCubeFaces);
    add_shader_constant_group(*registry, "frustum planes", kFrustumPlanes);
    add_shader_constant_group(*registry, "frustum corners", kFrustumCorners);
    add_shader_constant_group(*registry, "light types", kLightTypes);
    add_shader_constant_group(*registry, "lighting paths", kLightingPaths);
    add_shader_constant_group(
        *registry, "common struct sizes", kCommonStructSizes);
}

// Reconstructed from eboot.elf at 0x640D60.
void render_resource_manager_initialize_shader_constants(
    RenderResourceManager& manager) {
    auto& constants = manager.shader_constants;
    using Type = RenderShaderConstantType;

    constants.scene_block = render_shader_constant_block_create(
        "Scene", 0, 9, 5);
    auto& scene = *constants.scene_block;
    constants.time = render_shader_constant_block_add(
        scene, Type::vector4, "gTime");
    constants.smoothness_decay = render_shader_constant_block_add(
        scene, Type::scalar, "gSmoothnessDecay");
    constants.sgraph_trans_infos = render_shader_constant_block_add_array(
        scene, Type::matrix3x4, 4, "gSGraphTransInfos");
    constants.scene_global_floats = render_shader_constant_block_add_array(
        scene, Type::vector4, 1, "gSceneGlobalFloats");
    constants.scene_global_colors = render_shader_constant_block_add_array(
        scene, Type::vector4, 4, "gSceneGlobalColors");
    constants.tiled_lighting_params = render_shader_constant_block_add(
        scene, Type::vector3, "gTiledLightingParams");
    constants.fog_params = render_shader_constant_block_add(
        scene, Type::vector3, "gFogParams");
    constants.volumetric_params_0 = render_shader_constant_block_add(
        scene, Type::vector3, "gVolumetricParams0");
    constants.volumetric_params_1 = render_shader_constant_block_add(
        scene, Type::vector2, "gVolumetricParams1");

    constants.render_target_block = render_shader_constant_block_create(
        "RenderTarget", 1, 28, 80);
    constants.target_dimensions = render_shader_constant_block_add(
        *constants.render_target_block,
        Type::vector2,
        "gTargetDimensions");

    constants.camera_block = render_shader_constant_block_create(
        "Camera", 2, 29, 40);
    auto& camera = *constants.camera_block;
    constants.camera_near_far_params = render_shader_constant_block_add(
        camera, Type::vector4, "gCameraNearFarParams");
    constants.camera_misc_params = render_shader_constant_block_add(
        camera, Type::vector4, "gCameraMiscParams");
    constants.camera_view_extents = render_shader_constant_block_add_array(
        camera, Type::vector4, 4, "gCameraViewExtents");
    constants.camera_rt_sliced_data =
        render_shader_constant_block_add_sliced_array(
            camera, Type::vector4, 10, "gCameraRTSlicedData");

    constants.clip_planes_block = render_shader_constant_block_create(
        "ClipPlanes", 3, 1, 10);
    constants.clip_planes = render_shader_constant_block_add_array(
        *constants.clip_planes_block,
        Type::vector4,
        4,
        "gClipPlanes");

    constants.skeleton_block = render_shader_constant_block_create(
        "Skeleton", 4, 1, 1);
    constants.skeleton_bone_transforms =
        render_shader_constant_block_add_array(
            *constants.skeleton_block,
            Type::matrix3x4,
            256,
            "gSkeletonBoneXfms");

    constants.misc_draw_state_block = render_shader_constant_block_create(
        "MiscDrawState", 5, 8, 10);
    constants.environment_index = render_shader_constant_block_add(
        *constants.misc_draw_state_block,
        Type::scalar,
        "gEnvironIndex");
    constants.solid_color = render_shader_constant_block_add(
        *constants.misc_draw_state_block,
        Type::vector4,
        "gSolidColor");

    constants.occlusion_query_block = render_shader_constant_block_create(
        "OcclusionQuery", 6, 9, 10);
    constants.occlusion_query_coverage = render_shader_constant_block_add(
        *constants.occlusion_query_block,
        Type::vector2,
        "gOcclusionQueryCoverageParams");

    constants.debug_block = render_shader_constant_block_create(
        "Debug", 7, 24, 10);
    auto& debug = *constants.debug_block;
    constants.debug_modes = render_shader_constant_block_add(
        debug, Type::vector2, "gDebugModes");
    constants.debug_color = render_shader_constant_block_add(
        debug, Type::vector4, "gDebugColor");
    constants.batch_info = render_shader_constant_block_add(
        debug, Type::vector2, "gBatchInfo");
    constants.preview_node_index = render_shader_constant_block_add(
        debug, Type::scalar, "gPreviewNodeIndex");

    constexpr std::uint64_t kTransientCounts[] = {16, 32, 64};
    for (std::size_t index = 0; index < 3; ++index) {
        auto*& transient = constants.transient_blocks[index];
        transient = render_shader_constant_block_create(
            "Transient", 8, 29, 10);
        render_shader_constant_block_add_array(
            *transient,
            Type::vector4,
            kTransientCounts[index],
            "gTransientData");
    }

    constexpr std::uint32_t kFnv1aOffsetBasis = 0x811C9DC5U;
    std::uint32_t source_hash = kFnv1aOffsetBasis;
    render_shader_constant_block_accumulate_source_hash(
        *constants.scene_block, source_hash);
    render_shader_constant_registry_accumulate_source_hash(
        *constants.constant_registry, source_hash);

    RenderShaderConstantBlock* remaining_blocks[] = {
        constants.render_target_block,
        constants.camera_block,
        constants.clip_planes_block,
        constants.skeleton_block,
        constants.misc_draw_state_block,
        constants.debug_block,
    };
    for (const auto* block : remaining_blocks) {
        render_shader_constant_block_accumulate_source_hash(
            *block, source_hash);
    }
    manager.runtime.reserved_680 =
        (manager.runtime.reserved_680 & 0xFFFFFFFF00000000ULL) |
        source_hash;
}

// Reconstructed from eboot.elf at 0x63F400.
void render_resource_manager_initialize(RenderResourceManager& manager) {
    manager.shader_constants.initialization_phases[0] = 1;
    render_resource_manager_initialize_shader_constant_registry(manager);
    render_resource_manager_initialize_shader_parameters(manager);
    render_resource_manager_initialize_shader_constants(manager);

    auto& resources = manager.runtime.resources;
    resources.error_shader = create_builtin_shader(
        328, render_error_shader_construct);
    resources.basic_shader = create_builtin_shader(
        400, render_basic_shader_construct);
    resources.bink_convert_shader = create_builtin_shader(
        392, render_bink_convert_shader_construct);
    resources.bloom_shader = create_builtin_shader(
        376, render_bloom_shader_construct);
    resources.blur_shader = create_builtin_shader(
        480, render_blur_shader_construct);
    resources.fxaa_shader = create_builtin_shader(
        312, render_fxaa_shader_construct);
    resources.display_shading_mode_shader = create_builtin_shader(
        312, render_display_shading_mode_shader_construct);
    resources.display_sphere_map_shader = create_builtin_shader(
        296, render_display_sphere_map_shader_construct);
    resources.display_texture_cube_shader = create_builtin_shader(
        416, render_display_texture_cube_shader_construct);
    resources.downsample_shader = create_builtin_shader(
        376, render_downsample_shader_construct);
    resources.linearize_depth_shader = create_builtin_shader(
        296, render_linearize_depth_shader_construct);
    resources.output_conversion_shader = create_builtin_shader(
        384, render_output_conversion_shader_construct);
    resources.refine_scene_mask_shader = create_builtin_shader(
        296, render_refine_scene_mask_shader_construct);
    resources.stencil_scene_mask_shader = create_builtin_shader(
        312, render_stencil_scene_mask_shader_construct);
    resources.test_pattern_shader = create_builtin_shader(
        320, render_test_pattern_shader_construct);

    if (supports_async_compute()) {
        resources.blur_classify_compute_shader = create_builtin_shader(
            336, render_blur_classify_compute_shader_construct);
        resources.calc_depth_range_compute_shader = create_builtin_shader(
            320, render_calc_depth_range_compute_shader_construct);
        resources.clear_buffer_compute_shader = create_builtin_shader(
            392, render_clear_buffer_compute_shader_construct);
        resources.copy_buffer_compute_shader = create_builtin_shader(
            424, render_copy_buffer_compute_shader_construct);
        resources.dof_disc_blur_compute_shader = create_builtin_shader(
            376, render_dof_disc_blur_compute_shader_construct);
        resources.dof_sprite_shader = create_builtin_shader(
            304, render_dof_sprite_shader_construct);
        resources.vscat_density_compute_shader = create_builtin_shader(
            504, render_vscat_density_compute_shader_construct);
        resources.vscat_accumulation_compute_shader = create_builtin_shader(
            432, render_vscat_accumulation_compute_shader_construct);
        resources.vscat_deferred_compute_shader = create_builtin_shader(
            408, render_vscat_deferred_compute_shader_construct);
        resources.ssao_compute_shader = create_builtin_shader(
            352, render_ssao_compute_shader_construct);
        resources.cmaa_edge_detect_compute_shader = create_builtin_shader(
            328, render_cmaa_edge_detect_compute_shader_construct);
        resources.cmaa_edge_prune_compute_shader = create_builtin_shader(
            320, render_cmaa_edge_prune_compute_shader_construct);
        resources.cmaa_shape_fit_compute_shader = create_builtin_shader(
            328, render_cmaa_shape_fit_compute_shader_construct);
        resources.cmaa_final_process_compute_shader = create_builtin_shader(
            328, render_cmaa_final_process_compute_shader_construct);
        resources.linearize_depth_compute_shader = create_builtin_shader(
            344, render_linearize_depth_compute_shader_construct);
        resources.signed_distance_compute_shader = create_builtin_shader(
            344, render_signed_distance_compute_shader_construct);
        resources.signed_distance_classify_compute_shader =
            create_builtin_shader(
                336,
                render_signed_distance_classify_compute_shader_construct);
    }

    resources.render_test_shader = create_builtin_shader(
        344, render_test_shader_construct);
    if (supports_async_compute()) {
        resources.render_test_compute_shader = create_builtin_shader(
            352, render_test_compute_shader_construct);
    }
}

// Reconstructed from eboot.elf at 0x63F350.
void render_resource_manager_destruct(RenderResourceManager& manager) {
    release_list_sentinel(manager.primary_list);
    release_list_sentinel(manager.secondary_list);

    if (manager.pointer_array != nullptr) {
        auto& array = *manager.pointer_array;
        if (array.begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(array.capacity) -
                reinterpret_cast<std::uint8_t*>(array.begin));
            engine_deallocate_sized(array.begin, byte_count);
        }
        render_release(manager.pointer_array);
        manager.pointer_array = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x641370.
void render_resource_manager_finalize(RenderResourceManager& manager) {
    manager.shader_constants.initialization_phases[1] = 1;
    for (auto* node = manager.primary_list->next;
         node != manager.primary_list;
         node = node->next) {
        render_primary_shader_finalize(
            render_primary_shader_from_link(*node));
    }

    constexpr std::uint32_t kFunctionCount = 4;
    constexpr std::uint32_t kSampleCount = 128;
    constexpr float kSampleStep = 1.0F / 127.0F;
    constexpr RenderDataFormatDescriptor kFunctionTableFormat{
        32,
        10,
        2,
        1,
        -1,
    };

    RenderTextureArray1DDescriptor descriptor;
    render_texture_array_1d_descriptor_construct(descriptor);
    descriptor.texture_state.name = "function_table";
    descriptor.texture_state.address_mode = static_cast<std::uint32_t>(
        render_texture_default_address_mode(6));
    descriptor.texture_state.filter_mode = static_cast<std::uint32_t>(
        render_texture_default_filter_mode(6));

    std::array<RenderTextureMipChainDescriptor, kFunctionCount> mip_chains;
    descriptor.mip_chains = {
        mip_chains.data(),
        mip_chains.data() + mip_chains.size(),
        mip_chains.data() + mip_chains.size(),
    };

    const auto data_format =
        render_data_format_resolve(kFunctionTableFormat, 7);
    const RenderTextureExtent3D extent{kSampleCount, 1, 1};
    std::array<RenderFloatPixel, kSampleCount> pixels;
    for (std::uint32_t function_index = 0;
         function_index < kFunctionCount;
         ++function_index) {
        for (std::uint32_t sample_index = 0;
             sample_index < kSampleCount;
             ++sample_index) {
            const auto sample = sample_function_table(
                function_index,
                static_cast<float>(sample_index) * kSampleStep);
            pixels[sample_index] = {sample, sample, sample, sample};
        }

        auto& mip = mip_chains[function_index];
        render_texture_mip_chain_descriptor_construct(mip);
        render_texture_mip_chain_descriptor_allocate_source(
            mip, extent, data_format, nullptr);
        const RenderFloatImageView image{
            nullptr,
            kSampleCount,
            1,
            1,
            0,
            pixels.data(),
            nullptr,
        };
        render_texture_mip_chain_descriptor_copy_float_image(mip, image);
    }

    manager.runtime.function_table_texture =
        render_create_texture_array_1d(descriptor, nullptr);

    for (auto& mip : mip_chains) {
        render_texture_mip_chain_descriptor_destruct(mip);
    }
}

// Reconstructed from eboot.elf at 0x641740.
void render_resource_manager_shutdown(RenderResourceManager& manager) {
    RenderShaderConstantBlock** constant_blocks[] = {
        &manager.shader_constants.scene_block,
        &manager.shader_constants.render_target_block,
        &manager.shader_constants.camera_block,
        &manager.shader_constants.clip_planes_block,
        &manager.shader_constants.skeleton_block,
        &manager.shader_constants.misc_draw_state_block,
        &manager.shader_constants.occlusion_query_block,
        &manager.shader_constants.debug_block,
    };
    for (auto** block : constant_blocks) {
        render_shader_constant_block_release(*block);
    }
    for (auto*& storage : manager.shader_constants.transient_blocks) {
        render_shader_constant_block_release(storage);
    }

    render_shader_constant_registry_release(
        manager.shader_constants.constant_registry);

    if (manager.runtime.shader_parameters != nullptr) {
        auto* parameters = manager.runtime.shader_parameters;
        for (std::size_t index = 6; index != 0; --index) {
            destruct_parameter_registry(
                parameters->registries[index - 1]);
        }
        render_release(parameters);
        manager.runtime.shader_parameters = nullptr;
    }

    release_dynamic_resource(manager.runtime.function_table_texture);
    auto** resources = &manager.runtime.resources.error_shader;
    for (std::size_t index = 0; index < 35; ++index) {
        release_dynamic_resource(resources[index]);
    }
}

// Reconstructed from eboot.elf at 0x641F30, with the primary-resource clear
// helper at 0x6388C0 and compiled-array clear at 0x63B210.
void render_resource_manager_reload_shaders(RenderResourceManager& manager) {
    for (auto* node = manager.primary_list->next;
         node != manager.primary_list;
         node = node->next) {
        render_primary_shader_clear_compiled_objects(
            render_primary_shader_from_link(*node));
    }

    const auto& settings = *render_system_settings(*render_system_instance());
    if (settings.max_partial_framerate_scenes != 0) {
        return;
    }

    for (auto* node = manager.secondary_list->next;
         node != manager.secondary_list;
         node = node->next) {
        auto* bytes = reinterpret_cast<std::uint8_t*>(node);
        bytes[kSecondaryShaderDirtyOffset] = true;
    }
}

}  // namespace rb4
