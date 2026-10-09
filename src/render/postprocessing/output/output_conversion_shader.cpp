#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct OutputConversionShaderDispatch {
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

static_assert(sizeof(OutputConversionShaderDispatch) == 56);

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

// Reconstructed from eboot.elf at 0x636810.
void output_conversion_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

// Reconstructed from eboot.elf at 0x636820.
void output_conversion_shader_delete(void* shader) {
    output_conversion_shader_destruct(shader);
    render_release(shader);
}

// Reconstructed from eboot.elf at 0x636C40.
const void* output_conversion_shader_source_identifier(void*) {
    return "RndShaderOutputConversion";
}

// Reconstructed from eboot.elf at 0x636A50.
void* output_conversion_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/OutputConversion.hlsl");
}

// Reconstructed from eboot.elf at 0x636A60. The HMD-mask permutation is
// registered in the first registry; the color-space and transfer-function
// permutations use the pixel-stage registry.
void initialize_output_conversion_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    const Symbol hmd_mask("HX_USE_HMD_MASK");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        &parameters->registries[0],
        hmd_mask.value());
    auto* pixel_parameters = &parameters->registries[4];
    const Symbol color_space("HX_USE_BT709_TO_BT2020");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 1),
        pixel_parameters,
        color_space.value());
    const Symbol perceptual_quantizer("HX_USE_PERCEPTUAL_QUANTIZER");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 2),
        pixel_parameters,
        perceptual_quantizer.value());

    shader_field(shader, 352) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gMinIntensity");
    shader_field(shader, 360) = static_cast<std::int64_t>(
        constant_block->next_offset);
    shader_field(shader, 368) = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcTex", "gSrcTexSampler", 1, 4, 12);
    shader_field(shader, 376) = render_shader_backend_add_texture_binding(
        *backend_state, "gHmdMaskTex", "gHmdMaskTexSampler", 1, 4, 12);
}

std::int32_t output_conversion_shader_mode(void*) {
    return 0;
}

std::int32_t output_conversion_shader_variant(void*) {
    return 13;
}

OutputConversionShaderDispatch kOutputConversionShaderDispatch{
    output_conversion_shader_destruct,
    output_conversion_shader_delete,
    output_conversion_shader_source_identifier,
    output_conversion_shader_backend_path,
    initialize_output_conversion_shader_support_objects,
    output_conversion_shader_mode,
    output_conversion_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6367A0. The original dispatch table is at
// 0x192ED40.
void render_output_conversion_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kOutputConversionShaderDispatch;
    for (std::size_t index = 0; index < 3; ++index) {
        parameter_binding(shader, index) = {};
    }
    shader_field(shader, 352) = -1;
    shader_field(shader, 360) = 0;
    shader_field(shader, 368) = -1;
    shader_field(shader, 376) = -1;
}

}  // namespace rb4
