#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct VolumetricScatteringShaderDispatch {
    void (*destruct)(void* shader);
    void (*delete_resource)(void* shader);
    const void* (*source_identifier)(void* shader);
    void* (*backend_path)(void* shader);
    void (*initialize_support_objects)(
        void* shader,
        RenderShaderConstantRegistry* constants,
        RenderShaderParameterRegistrySet* parameters,
        RenderShaderConstantBlock* constant_block,
        RenderShaderBackendState* backend_state);
    std::int32_t (*mode)(void* shader);
    std::int32_t (*variant)(void* shader);
    bool (*validate_permutation)(
        void* shader,
        std::uint32_t stage,
        std::uint64_t key);
    void (*bind_fallback)(void* shader, void* context);
    bool (*supports_render_target_slices)(void* shader);
    bool (*uses_geometry_program)(void* shader);
};

static_assert(sizeof(VolumetricScatteringShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::uint8_t* shader_bytes(void* shader) {
    return static_cast<std::uint8_t*>(shader);
}

RenderShaderParameterBinding& parameter_binding(
    void* shader,
    std::size_t index) {
    return *reinterpret_cast<RenderShaderParameterBinding*>(
        shader_bytes(shader) + 288 + index * sizeof(RenderShaderParameterBinding));
}

std::int64_t& shader_field(void* shader, std::size_t offset) {
    return *reinterpret_cast<std::int64_t*>(shader_bytes(shader) + offset);
}

const RenderSettings& current_settings() {
    static const RenderSettings default_settings{};
    auto* system = render_system_instance();
    if (system == nullptr) {
        return default_settings;
    }
    auto* settings = render_system_settings(*system);
    return settings == nullptr ? default_settings : *settings;
}

void volumetric_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void volumetric_shader_delete(void* shader) {
    volumetric_shader_destruct(shader);
    MemFree(shader);
}

std::int32_t volumetric_shader_mode(void*) {
    return 0;
}

std::int32_t volumetric_shader_variant(void*) {
    return 16;
}

const void* accumulation_source_identifier(void*) {
    return "RndCShaderVScatAccumScattering";
}

void* accumulation_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/VScatAccumScattering.hlsl");
}

void initialize_accumulation_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    const auto tile_size = static_cast<std::int32_t>(
        current_settings().light_tile_size);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TILE_SIZE", tile_size);

    const Symbol use_scene_mask("HX_USE_SCENE_MASK");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        &parameters->registries[5],
        use_scene_mask.Str());
    const Symbol is_stereo("HX_IS_STEREO");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 1),
        &parameters->registries[5],
        is_stereo.Str());
    const Symbol stereo_eye("HX_STEREO_EYE");
    render_shader_parameter_registry_add(
        &parameter_binding(shader, 2),
        &parameters->registries[5],
        stereo_eye.Str(),
        0,
        2);

    shader_field(shader, 352) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gDensityInscatteringTex",
        "gDensityInscatteringTexSampler",
        2,
        5,
        12);
    shader_field(shader, 360) = render_shader_backend_add_texture_binding(
        *backend_state, "gTiledDepthRangeBuffer", "", 1, 5, 12);
    shader_field(shader, 368) = render_shader_backend_add_texture_binding(
        *backend_state, "gSceneMask", "", 1, 5, 12);
    shader_field(shader, 376) = render_shader_backend_add_output_binding(
        *backend_state, "gOutputTex", 2, 5, 12);
    shader_field(shader, 384) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector3,
        "gSrcDimensions");
    shader_field(shader, 392) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector3,
        "gDstDimensions");
    shader_field(shader, 400) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gScreenDimensions");
    shader_field(shader, 408) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gDepthFracOffsetParams");
    shader_field(shader, 416) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gFrustumParams");
    shader_field(shader, 424) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

const void* density_source_identifier(void*) {
    return "RndCShaderVScatCalcDensityInscattering";
}

void* density_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/"
        "VScatCalcDensityInscattering.hlsl");
}

void initialize_density_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    const auto& settings = current_settings();
    render_shader_constant_registry_add_definition(
        *constants,
        "HX_TILE_SIZE",
        static_cast<std::int32_t>(settings.light_tile_size));
    render_shader_constant_registry_add_definition(
        *constants,
        "HX_TILE_DEPTH_SLICES",
        static_cast<std::int32_t>(settings.light_tile_depth_slices));
    render_shader_constant_registry_add_definition(
        *constants, "HX_SHADOW_CAST_CONTEXT_VOLUMETRIC", 1);

    const Symbol color_space("HX_BT709_TO_BT2020");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        &parameters->registries[5],
        color_space.Str());

    shader_field(shader, 312) = render_shader_backend_add_output_binding(
        *backend_state, "gOutputTex", 2, 5, 12);
    shader_field(shader, 320) = render_shader_backend_add_texture_binding(
        *backend_state, "gSkyTex", "gSkyTexSampler", 1, 5, 12);
    shader_field(shader, 328) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gFunctionTable",
        "gFunctionTableSampler",
        4,
        5,
        12);
    shader_field(shader, 336) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gSpotShadowMapArray",
        "gSpotShadowMapArraySampler",
        5,
        5,
        12);
    shader_field(shader, 344) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gHeightWaveform",
        "gHeightWaveformSampler",
        0,
        5,
        12);
    shader_field(shader, 352) =
        render_shader_backend_add_structured_buffer_input(
            *backend_state, "gPointLights", "CSLightPoint", 0, 5);
    shader_field(shader, 360) =
        render_shader_backend_add_structured_buffer_input(
            *backend_state, "gSpotLights", "CSLightSpot", 0, 5);
    shader_field(shader, 368) =
        render_shader_backend_add_structured_buffer_input(
            *backend_state,
            "gDirectionalLights",
            "CSLightDirectional",
            0,
            5);
    shader_field(shader, 376) =
        render_shader_backend_add_structured_buffer_input(
            *backend_state, "gLightIds", "Uint2As32", 0, 5);
    shader_field(shader, 384) =
        render_shader_backend_add_structured_buffer_input(
            *backend_state,
            "gLightIdRanges",
            "CSLightIdRange",
            0,
            5);
    shader_field(shader, 392) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::boolean, "gIsStereo");
    shader_field(shader, 400) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::scalar,
        "gNumDirectionalLights");
    shader_field(shader, 408) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gFogDensity");
    shader_field(shader, 416) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::scalar,
        "gVolumetricLightingIntensity");
    shader_field(shader, 424) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gDepthFracOffsetParams");
    shader_field(shader, 432) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gDimensions");
    shader_field(shader, 440) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector3, "gVolumeDim");
    shader_field(shader, 448) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gSkyTexDim");
    shader_field(shader, 456) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gTileCounts");
    shader_field(shader, 464) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gHeightParams");
    shader_field(shader, 472) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gCamNearFarParams");
    shader_field(shader, 480) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::matrix3x4,
        "gCamWorldXfmInv");
    shader_field(shader, 488) = render_shader_constant_block_add_array(
        *constant_block,
        RenderShaderConstantType::vector3,
        8,
        "gFrustumCorners");
    shader_field(shader, 496) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

const void* deferred_source_identifier(void*) {
    return "RndCShaderVScatDeferred";
}

void* deferred_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/VScatDeferred.hlsl");
}

void initialize_deferred_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    render_shader_constant_registry_add_definition(
        *constants,
        "HX_TILE_SIZE",
        static_cast<std::int32_t>(current_settings().light_tile_size));
    const Symbol use_scene_mask("HX_USE_SCENE_MASK");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        &parameters->registries[5],
        use_scene_mask.Str());

    shader_field(shader, 312) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gAccumScatteringTex",
        "gAccumScatteringTexSampler",
        2,
        5,
        12);
    shader_field(shader, 320) = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcLightAccumBuffer", "", 1, 5, 12);
    shader_field(shader, 328) = render_shader_backend_add_texture_binding(
        *backend_state, "gLinearDepthTex", "", 1, 5, 12);
    shader_field(shader, 336) = render_shader_backend_add_texture_binding(
        *backend_state, "gSceneMask", "", 1, 5, 12);
    shader_field(shader, 344) = render_shader_backend_add_output_binding(
        *backend_state, "gDstLightAccumBuffer", 1, 5, 12);
    shader_field(shader, 352) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gDimensions");
    shader_field(shader, 360) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::matrix4x4, "gProj");
    shader_field(shader, 368) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gDepthFracOffsetParams");
    shader_field(shader, 376) = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::scalar,
        "gFogEndDistance");
    shader_field(shader, 384) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gFogDensity");
    shader_field(shader, 392) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector3, "gTexelSize");
    shader_field(shader, 400) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

VolumetricScatteringShaderDispatch kAccumulationDispatch{
    volumetric_shader_destruct,
    volumetric_shader_delete,
    accumulation_source_identifier,
    accumulation_backend_path,
    initialize_accumulation_support_objects,
    volumetric_shader_mode,
    volumetric_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

VolumetricScatteringShaderDispatch kDensityDispatch{
    volumetric_shader_destruct,
    volumetric_shader_delete,
    density_source_identifier,
    density_backend_path,
    initialize_density_support_objects,
    volumetric_shader_mode,
    volumetric_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

VolumetricScatteringShaderDispatch kDeferredDispatch{
    volumetric_shader_destruct,
    volumetric_shader_delete,
    deferred_source_identifier,
    deferred_backend_path,
    initialize_deferred_support_objects,
    volumetric_shader_mode,
    volumetric_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

void construct_parameterized_shader(
    void* shader,
    VolumetricScatteringShaderDispatch& dispatch,
    std::size_t binding_count) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &dispatch;
    for (std::size_t index = 0; index < binding_count; ++index) {
        parameter_binding(shader, index) = {};
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D1E70.
void render_vscat_accumulation_compute_shader_construct(void* shader) {
    construct_parameterized_shader(shader, kAccumulationDispatch, 3);
    for (std::size_t offset = 352; offset <= 416; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 424) = 0;
}

// Reconstructed from eboot.elf at 0x6D26D0.
void render_vscat_density_compute_shader_construct(void* shader) {
    construct_parameterized_shader(shader, kDensityDispatch, 1);
    for (std::size_t offset = 312; offset <= 488; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 496) = 0;
}

// Reconstructed from eboot.elf at 0x6D3490.
void render_vscat_deferred_compute_shader_construct(void* shader) {
    construct_parameterized_shader(shader, kDeferredDispatch, 1);
    shader_field(shader, 312) = -1;
    shader_field(shader, 320) = -1;
    shader_field(shader, 328) = -1;
    for (std::size_t offset = 344; offset <= 392; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 400) = 0;
}

}  // namespace rb4
