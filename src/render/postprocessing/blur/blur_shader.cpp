#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct BlurShaderDispatch {
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
};

static_assert(sizeof(BlurShaderDispatch) == 56);

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

// Reconstructed from eboot.elf at 0x634B80.
void blur_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

// Reconstructed from eboot.elf at 0x634B90.
void blur_shader_delete(void* shader) {
    blur_shader_destruct(shader);
    render_release(shader);
}

// Reconstructed from eboot.elf at 0x635FB0.
const void* blur_shader_source_identifier(void*) {
    return "RndShaderBlur";
}

// Reconstructed from eboot.elf at 0x635AF0.
void* blur_shader_backend_path(void*) {
    return const_cast<char*>("../../system/data/shaders/Blur.hlsl");
}

// Reconstructed from eboot.elf at 0x635B00.
void initialize_blur_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* pixel_parameters = &parameters->registries[4];
    const Symbol texture_array("HX_IS_TEX_ARRAY");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        pixel_parameters,
        texture_array.value());
    const Symbol sample_count("HX_NUM_BLUR_SAMPLES");
    render_shader_parameter_registry_add(
        &parameter_binding(shader, 1),
        pixel_parameters,
        sample_count.value(),
        0,
        33);
    const Symbol blur_type("HX_BLUR_TYPE");
    render_shader_parameter_registry_add(
        &parameter_binding(shader, 2),
        pixel_parameters,
        blur_type.value(),
        0,
        2);
    const Symbol blur_direction("HX_BLUR_DIRECTION");
    render_shader_parameter_registry_add(
        &parameter_binding(shader, 3),
        pixel_parameters,
        blur_direction.value(),
        0,
        2);
    const Symbol scene_mask("HX_USE_SCENE_MASK");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 4),
        pixel_parameters,
        scene_mask.value());
    const Symbol classification("HX_USE_CLASSIFICATION_BUFFER");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 5),
        pixel_parameters,
        classification.value());

    render_shader_constant_registry_add_definition(
        *constants, "HX_BLUR_TYPE_GAUSSIAN", 0);
    render_shader_constant_registry_add_definition(
        *constants, "HX_BLUR_TYPE_DEPTH_AWARE", 1);
    render_shader_constant_registry_add_definition(
        *constants, "HX_BLUR_DIRECTION_HORIZONTAL", 0);
    render_shader_constant_registry_add_definition(
        *constants, "HX_BLUR_DIRECTION_VERTICAL", 1);
    static const RenderSettings default_settings{};
    auto* system = render_system_instance();
    auto* settings = system == nullptr ? nullptr : render_system_settings(*system);
    const auto tile_size = static_cast<std::int32_t>(
        (settings == nullptr ? default_settings : *settings).light_tile_size);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TILE_SIZE", tile_size);

    shader_field(shader, 408) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gArraySlice");
    shader_field(shader, 416) = render_shader_constant_block_add_array(
        *constant_block,
        RenderShaderConstantType::vector3,
        32,
        "gBlurSampleOffsetsWeights");
    shader_field(shader, 424) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gMaxOffset");
    shader_field(shader, 432) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector3, "gTileInfo");
    shader_field(shader, 440) = static_cast<std::int64_t>(
        constant_block->next_offset);

    shader_field(shader, 448) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexture", "gTexSampler", 1, 4, 12);
    shader_field(shader, 456) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexArray", "gTexArraySampler", 5, 4, 12);
    shader_field(shader, 464) = render_shader_backend_add_texture_binding(
        *backend_state, "gSceneMask", "gSceneMaskSampler", 1, 4, 12);
    shader_field(shader, 472) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gTiledClassificationBuffer",
        "gTiledClassificationSampler",
        1,
        4,
        12);
}

std::int32_t blur_shader_mode(void*) {
    return 0;
}

std::int32_t blur_shader_variant(void*) {
    return 13;
}

BlurShaderDispatch kBlurShaderDispatch{
    blur_shader_destruct,
    blur_shader_delete,
    blur_shader_source_identifier,
    blur_shader_backend_path,
    initialize_blur_shader_support_objects,
    blur_shader_mode,
    blur_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x634AE0. The original dispatch table is at
// 0x192EC08. The constructor intentionally leaves the scene-mask handle at
// offset 464 untouched; support initialization assigns it.
void render_blur_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kBlurShaderDispatch;
    for (std::size_t index = 0; index < 6; ++index) {
        parameter_binding(shader, index) = {};
    }
    for (std::size_t offset = 408; offset <= 432; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 440) = 0;
    shader_field(shader, 448) = -1;
    shader_field(shader, 456) = -1;
    shader_field(shader, 472) = -1;
}

}  // namespace rb4
