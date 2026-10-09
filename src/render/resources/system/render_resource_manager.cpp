#include "render/resources/system/render_resource_manager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>

#include "math/color/Color.h"
#include "math/vector/Vector3i.h"
#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "utl/text/Symbol.h"
#include "render/core/settings/render_settings.h"
#include "render/core/platform/render_platform_config.h"
#include "render/system/RndDevice.h"
#include "render/core/textures/render_data_format.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelData.h"
#include "utl/text/Str.h"
#include "render/shaders/RndShader.h"
#include "render/core/buffers/RndCShaderClearBuffer.h"
#include "render/core/buffers/RndCShaderCopyBuffer.h"
#include "render/core/debug/RndCShaderRenderTestCompute.h"
#include "render/core/debug/RndShaderDisplayShadingMode.h"
#include "render/core/debug/RndShaderDisplaySphereMap.h"
#include "render/core/debug/RndShaderDisplayTextureCube.h"
#include "render/core/debug/RndShaderRenderTestSimple.h"
#include "render/core/debug/RndShaderTestPattern.h"
#include "render/depth/RndCShaderCalcDepthRange.h"
#include "render/depth/RndCShaderLinearizeDepth.h"
#include "render/depth/RndShaderLinearizeDepth.h"
#include "render/distance_fields/RndCShaderSignedDistance.h"
#include "render/distance_fields/RndCShaderSignedDistanceClassify.h"
#include "render/lighting/ambient_occlusion/RndCShaderSSAOGen.h"
#include "render/lighting/volumetric/RndCShaderVScatAccumScattering.h"
#include "render/lighting/volumetric/RndCShaderVScatCalcDensityInscattering.h"
#include "render/lighting/volumetric/RndCShaderVScatDeferred.h"
#include "render/masking/RndShaderRefineSceneMask.h"
#include "render/masking/RndShaderStencilSceneMask.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgeDetect.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgePrune.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAFinalProcess.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAShapeFit.h"
#include "render/postprocessing/antialiasing/RndShaderFXAA.h"
#include "render/postprocessing/bloom/RndShaderBloom.h"
#include "render/postprocessing/blur/RndCShaderBlurClassify.h"
#include "render/postprocessing/blur/RndShaderBlur.h"
#include "render/postprocessing/depth_of_field/RndCShaderDOFDiscBlur.h"
#include "render/postprocessing/depth_of_field/RndShaderDOFSprite.h"
#include "render/postprocessing/downsample/RndShaderDownsample.h"
#include "render/postprocessing/output/RndShaderOutputConversion.h"
#include "render/resources/video/RndShaderBinkConvert.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderError.h"

namespace rb4 {

namespace {

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

RndShaderLink* create_list_sentinel() {
    auto* node = new RndShaderLink;
    node->mNext = node;
    node->mPrev = node;
    return node;
}

void release_list_sentinel(RndShaderLink*& node) {
    if (node == nullptr) {
        return;
    }

    node->mNext->mPrev = node->mPrev;
    node->mPrev->mNext = node->mNext;
    delete node;
    node = nullptr;
}

void release_dynamic_resource(void*& storage) {
    auto* resource = static_cast<RenderManagedObject*>(storage);
    if (resource != nullptr) {
        resource->dispatch->release_dynamic(resource);
        storage = nullptr;
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
    RndShaderFixedDefines& registry,
    const char* comment,
    const ShaderConstantDefinition (&definitions)[Count]) {
    registry.AddComment(comment);
    for (const auto& definition : definitions) {
        registry.Add(Symbol(definition.name), definition.value);
    }
}

template <typename T>
T* NewShader() {
    auto* shader = new T;
    shader->_Register();
    return shader;
}

template <typename T>
void Release(T*& object) {
    if (object != nullptr) {
        delete object;
        object = nullptr;
    }
}

bool supports_async_compute() {
    const auto& platform = TheRndDevice()->mPlatformConfigs[kCurrentPlatformConfigIndex];
    return (platform.feature_flags & kAsyncComputeFeature) != 0;
}

}  // namespace

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

    manager.shader_cache_defines = static_cast<RenderShaderCacheDefineArray*>(
        operator new(sizeof(RenderShaderCacheDefineArray)));
    *manager.shader_cache_defines = {};
    manager.primary_list = create_list_sentinel();
    manager.secondary_list = create_list_sentinel();
}

// Reconstructed from eboot.elf at 0x640BF0.
void render_resource_manager_initialize_shader_parameters(
    RenderResourceManager& manager) {
    auto* parameters = new RndShaderDefinesGroup;
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
        manager.shader_parameter_bindings[index] =
            parameters->mDefines[definition.registry_index].Add(
                name,
                static_cast<int>(definition.first_value),
                static_cast<int>(definition.last_value));
    }
}

// Reconstructed from eboot.elf at 0x63F920.
void render_resource_manager_initialize_shader_constant_registry(
    RenderResourceManager& manager) {
    auto* registry = new RndShaderFixedDefines;
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

    constants.scene_block = new RndShaderCBufferConfig("Scene", 0, 9, 5);
    auto& scene = *constants.scene_block;
    constants.time = scene.AddConstant(kShaderNumericFloat4, "gTime");
    constants.smoothness_decay = scene.AddConstant(kShaderNumericFloat, "gSmoothnessDecay");
    constants.sgraph_trans_infos = scene.AddConstantArray(kShaderNumericFloat3x4, 4, "gSGraphTransInfos");
    constants.scene_global_floats = scene.AddConstantArray(kShaderNumericFloat4, 1, "gSceneGlobalFloats");
    constants.scene_global_colors = scene.AddConstantArray(kShaderNumericFloat4, 4, "gSceneGlobalColors");
    constants.tiled_lighting_params = scene.AddConstant(kShaderNumericFloat3, "gTiledLightingParams");
    constants.fog_params = scene.AddConstant(kShaderNumericFloat3, "gFogParams");
    constants.volumetric_params_0 = scene.AddConstant(kShaderNumericFloat3, "gVolumetricParams0");
    constants.volumetric_params_1 = scene.AddConstant(kShaderNumericFloat2, "gVolumetricParams1");

    constants.render_target_block = new RndShaderCBufferConfig("RenderTarget", 1, 28, 80);
    constants.target_dimensions = constants.render_target_block->AddConstant(kShaderNumericFloat2, "gTargetDimensions");

    constants.camera_block = new RndShaderCBufferConfig("Camera", 2, 29, 40);
    auto& camera = *constants.camera_block;
    constants.camera_near_far_params = camera.AddConstant(kShaderNumericFloat4, "gCameraNearFarParams");
    constants.camera_misc_params = camera.AddConstant(kShaderNumericFloat4, "gCameraMiscParams");
    constants.camera_view_extents = camera.AddConstantArray(kShaderNumericFloat4, 4, "gCameraViewExtents");
    constants.camera_rt_sliced_data =
        camera.AddRTSlicedConstantArray(kShaderNumericFloat4, 10, "gCameraRTSlicedData");

    constants.clip_planes_block = new RndShaderCBufferConfig("ClipPlanes", 3, 1, 10);
    constants.clip_planes = constants.clip_planes_block->AddConstantArray(kShaderNumericFloat4, 4, "gClipPlanes");

    constants.skeleton_block = new RndShaderCBufferConfig("Skeleton", 4, 1, 1);
    constants.skeleton_bone_transforms =
        constants.skeleton_block->AddConstantArray(kShaderNumericFloat3x4, 256, "gSkeletonBoneXfms");

    constants.misc_draw_state_block = new RndShaderCBufferConfig("MiscDrawState", 5, 8, 10);
    constants.environment_index = constants.misc_draw_state_block->AddConstant(kShaderNumericFloat, "gEnvironIndex");
    constants.solid_color = constants.misc_draw_state_block->AddConstant(kShaderNumericFloat4, "gSolidColor");

    constants.occlusion_query_block = new RndShaderCBufferConfig("OcclusionQuery", 6, 9, 10);
    constants.occlusion_query_coverage = constants.occlusion_query_block->AddConstant(kShaderNumericFloat2, "gOcclusionQueryCoverageParams");

    constants.debug_block = new RndShaderCBufferConfig("Debug", 7, 24, 10);
    auto& debug = *constants.debug_block;
    constants.debug_modes = debug.AddConstant(kShaderNumericFloat2, "gDebugModes");
    constants.debug_color = debug.AddConstant(kShaderNumericFloat4, "gDebugColor");
    constants.batch_info = debug.AddConstant(kShaderNumericFloat2, "gBatchInfo");
    constants.preview_node_index = debug.AddConstant(kShaderNumericFloat, "gPreviewNodeIndex");

    constexpr std::uint64_t kTransientCounts[] = {16, 32, 64};
    for (std::size_t index = 0; index < 3; ++index) {
        auto*& transient = constants.transient_blocks[index];
        transient = new RndShaderCBufferConfig("Transient", 8, 29, 10);
        transient->AddConstantArray(kShaderNumericFloat4, kTransientCounts[index], "gTransientData");
    }

    constexpr std::uint32_t kFnv1aOffsetBasis = 0x811C9DC5U;
    std::uint32_t source_hash = kFnv1aOffsetBasis;
    constants.scene_block->PrintCode(source_hash);
    constants.constant_registry->PrintCode(source_hash);

    RndShaderCBufferConfig* remaining_blocks[] = {
        constants.render_target_block,
        constants.camera_block,
        constants.clip_planes_block,
        constants.skeleton_block,
        constants.misc_draw_state_block,
        constants.debug_block,
    };
    for (const auto* block : remaining_blocks) {
        block->PrintCode(source_hash);
    }
    manager.runtime.constant_source_hash =
        (manager.runtime.constant_source_hash & 0xFFFFFFFF00000000ULL) |
        source_hash;
}

// Reconstructed from eboot.elf at 0x63F400.
void render_resource_manager_initialize(RenderResourceManager& manager) {
    manager.shader_constants.initialization_phases[0] = 1;
    render_resource_manager_initialize_shader_constant_registry(manager);
    render_resource_manager_initialize_shader_parameters(manager);
    render_resource_manager_initialize_shader_constants(manager);

    auto& resources = manager.runtime.resources;
    resources.error_shader = NewShader<RndShaderError>();
    resources.basic_shader = NewShader<RndShaderBasic>();
    resources.bink_convert_shader = NewShader<RndShaderBinkConvert>();
    resources.bloom_shader = NewShader<RndShaderBloom>();
    resources.blur_shader = NewShader<RndShaderBlur>();
    resources.fxaa_shader = NewShader<RndShaderFXAA>();
    resources.display_shading_mode_shader = NewShader<RndShaderDisplayShadingMode>();
    resources.display_sphere_map_shader = NewShader<RndShaderDisplaySphereMap>();
    resources.display_texture_cube_shader = NewShader<RndShaderDisplayTextureCube>();
    resources.downsample_shader = NewShader<RndShaderDownsample>();
    resources.linearize_depth_shader = NewShader<RndShaderLinearizeDepth>();
    resources.output_conversion_shader = NewShader<RndShaderOutputConversion>();
    resources.refine_scene_mask_shader = NewShader<RndShaderRefineSceneMask>();
    resources.stencil_scene_mask_shader = NewShader<RndShaderStencilSceneMask>();
    resources.test_pattern_shader = NewShader<RndShaderTestPattern>();

    if (supports_async_compute()) {
        resources.blur_classify_compute_shader = NewShader<RndCShaderBlurClassify>();
        resources.calc_depth_range_compute_shader = NewShader<RndCShaderCalcDepthRange>();
        resources.clear_buffer_compute_shader = NewShader<RndCShaderClearBuffer>();
        resources.copy_buffer_compute_shader = NewShader<RndCShaderCopyBuffer>();
        resources.dof_disc_blur_compute_shader = NewShader<RndCShaderDOFDiscBlur>();
        resources.dof_sprite_shader = NewShader<RndShaderDOFSprite>();
        resources.vscat_density_compute_shader = NewShader<RndCShaderVScatCalcDensityInscattering>();
        resources.vscat_accumulation_compute_shader = NewShader<RndCShaderVScatAccumScattering>();
        resources.vscat_deferred_compute_shader = NewShader<RndCShaderVScatDeferred>();
        resources.ssao_compute_shader = NewShader<RndCShaderSSAOGen>();
        resources.cmaa_edge_detect_compute_shader = NewShader<RndCShaderCMAAEdgeDetect>();
        resources.cmaa_edge_prune_compute_shader = NewShader<RndCShaderCMAAEdgePrune>();
        resources.cmaa_shape_fit_compute_shader = NewShader<RndCShaderCMAAShapeFit>();
        resources.cmaa_final_process_compute_shader = NewShader<RndCShaderCMAAFinalProcess>();
        resources.linearize_depth_compute_shader = NewShader<RndCShaderLinearizeDepth>();
        resources.signed_distance_compute_shader = NewShader<RndCShaderSignedDistance>();
        resources.signed_distance_classify_compute_shader = NewShader<RndCShaderSignedDistanceClassify>();
    }

    resources.render_test_shader = NewShader<RndShaderRenderTestSimple>();
    if (supports_async_compute()) {
        resources.render_test_compute_shader = NewShader<RndCShaderRenderTestCompute>();
    }
}

// Reconstructed from eboot.elf at 0x63F350.
void render_resource_manager_destruct(RenderResourceManager& manager) {
    release_list_sentinel(manager.primary_list);
    release_list_sentinel(manager.secondary_list);

    if (manager.shader_cache_defines != nullptr) {
        auto& array = *manager.shader_cache_defines;
        if (array.begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(array.capacity) -
                reinterpret_cast<std::uint8_t*>(array.begin));
            HmxAllocator::gStlAllocator.deallocate(array.begin, byte_count);
        }
        MemFree(manager.shader_cache_defines);
        manager.shader_cache_defines = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x641370.
void render_resource_manager_finalize(RenderResourceManager& manager) {
    manager.shader_constants.initialization_phases[1] = 1;
    for (auto* node = manager.primary_list->mNext;
         node != manager.primary_list;
         node = node->mNext) {
        RndShader::FromLink(node)->Init();
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

    RndTextureArray1D::Description descriptor;
    descriptor.mName = "function_table";
    descriptor.mFormat.mWrapMode = static_cast<std::uint32_t>(
        TextureDefaultWrapMode(6));
    descriptor.mFormat.mFilterMode = static_cast<std::uint32_t>(
        TextureDefaultFilterMode(6));

    std::array<RndPixelData, kFunctionCount> mip_chains;
    descriptor.mPixels = {
        mip_chains.data(),
        mip_chains.data() + mip_chains.size(),
        mip_chains.data() + mip_chains.size(),
    };

    const auto data_format =
        render_data_format_resolve(kFunctionTableFormat, 7);
    const Vector3i extent{static_cast<int>(kSampleCount), 1, 1};
    std::array<Hmx::Color, kSampleCount> pixels;
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
        mip.Create(extent, data_format, nullptr);
        const RndPixelCanvas image{
            nullptr,
            static_cast<int>(kSampleCount),
            1,
            1,
            0,
            pixels.data(),
            nullptr,
        };
        mip.ConvertFrom(image);
    }

    manager.runtime.function_table_texture =
        RndTextureArray1D::New(descriptor, nullptr);
}

// Reconstructed from eboot.elf at 0x641740.
void render_resource_manager_shutdown(RenderResourceManager& manager) {
    RndShaderCBufferConfig** constant_blocks[] = {
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
        Release(*block);
    }
    for (auto*& block : manager.shader_constants.transient_blocks) {
        Release(block);
    }
    Release(manager.shader_constants.constant_registry);
    Release(manager.runtime.shader_parameters);

    release_dynamic_resource(manager.runtime.function_table_texture);
    auto& r = manager.runtime.resources;
    Release(r.error_shader);
    Release(r.basic_shader);
    Release(r.bink_convert_shader);
    Release(r.bloom_shader);
    Release(r.blur_shader);
    Release(r.fxaa_shader);
    Release(r.dof_sprite_shader);
    Release(r.display_shading_mode_shader);
    Release(r.display_sphere_map_shader);
    Release(r.display_texture_cube_shader);
    Release(r.downsample_shader);
    Release(r.linearize_depth_shader);
    Release(r.output_conversion_shader);
    Release(r.refine_scene_mask_shader);
    Release(r.stencil_scene_mask_shader);
    Release(r.test_pattern_shader);
    release_dynamic_resource(r.reserved_16);
    Release(r.blur_classify_compute_shader);
    Release(r.calc_depth_range_compute_shader);
    Release(r.clear_buffer_compute_shader);
    Release(r.copy_buffer_compute_shader);
    Release(r.dof_disc_blur_compute_shader);
    Release(r.vscat_density_compute_shader);
    Release(r.vscat_accumulation_compute_shader);
    Release(r.vscat_deferred_compute_shader);
    Release(r.ssao_compute_shader);
    Release(r.cmaa_edge_detect_compute_shader);
    Release(r.cmaa_edge_prune_compute_shader);
    Release(r.cmaa_shape_fit_compute_shader);
    Release(r.cmaa_final_process_compute_shader);
    Release(r.linearize_depth_compute_shader);
    Release(r.signed_distance_compute_shader);
    Release(r.signed_distance_classify_compute_shader);
    Release(r.render_test_shader);
    Release(r.render_test_compute_shader);
}

// Reconstructed from eboot.elf at 0x641F30, with the primary-resource clear
// helper at 0x6388C0 and compiled-array clear at 0x63B210.
void render_resource_manager_reload_shaders(RenderResourceManager& manager) {
    for (auto* node = manager.primary_list->mNext;
         node != manager.primary_list;
         node = node->mNext) {
        RndShader::FromLink(node)->Reload();
    }

    const auto& settings = *TheRndDevice()->mSettings;
    if (settings.max_partial_framerate_scenes != 0) {
        return;
    }

    for (auto* node = manager.secondary_list->mNext;
         node != manager.secondary_list;
         node = node->mNext) {
        auto* bytes = reinterpret_cast<std::uint8_t*>(node);
        bytes[kSecondaryShaderDirtyOffset] = true;
    }
}

}  // namespace rb4
