#include "render/postprocessing/output/output_conversion_shader.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "core/color/color_space.h"
#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/resources/shaders/builtin_shader_resources.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_draw_state.h"
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
    bool (*validate_permutation)(
        void* shader,
        std::uint32_t stage,
        std::uint64_t key);
    void (*bind_fallback)(void* shader, void* context);
    bool (*supports_render_target_slices)(void* shader);
    bool (*uses_geometry_program)(void* shader);
};

static_assert(sizeof(OutputConversionShaderDispatch) == 88);

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

// Reconstructed from eboot.elf at 0x636BF0. HMD-mask permutations are never
// built, on any stage.
bool validate_output_conversion_permutation(
    void* shader,
    std::uint32_t,
    std::uint64_t key) {
    return render_shader_parameter_binding_value(
               parameter_binding(shader, 0), key) == 0;
}

OutputConversionShaderDispatch kOutputConversionShaderDispatch{
    output_conversion_shader_destruct,
    output_conversion_shader_delete,
    output_conversion_shader_source_identifier,
    output_conversion_shader_backend_path,
    initialize_output_conversion_shader_support_objects,
    output_conversion_shader_mode,
    output_conversion_shader_variant,
    validate_output_conversion_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
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

// Reconstructed from eboot.elf at 0x636840. Uploads the linearized minimum
// intensity before binding the source and optional HMD mask with binding flag
// 2. The HMD-mask permutation is global, so it is written into every program
// key; the color-space and transfer-function permutations are pixel-only.
void render_output_conversion_shader_draw(
    void* shader,
    RenderContext& context,
    const RenderOutputConversionDrawParameters& parameters) {
    constexpr std::size_t kPixelKey = 3;
    constexpr std::uint32_t kTextureFlags = 2;

    const auto extent = static_cast<std::uint64_t>(shader_field(shader, 360));
    auto& buffer = render_shader_select_constant_buffer(context, extent);
    const float intensity[4] = {
        parameters.minimum_intensity,
        parameters.minimum_intensity,
        parameters.minimum_intensity,
        1.0F,
    };
    float linear[4] = {0.0F, 0.0F, 0.0F, 1.0F};
    color_srgb_to_linear(intensity, linear);
    std::memcpy(
        render_shader_constant_member(buffer, shader_field(shader, 352)),
        &linear[0],
        sizeof(linear[0]));
    render_shader_commit_constant_buffer(buffer, context, extent);

    render_shader_bind_pixel_texture(
        context, parameters.source, shader_field(shader, 368), kTextureFlags);
    render_shader_bind_pixel_texture(
        context, parameters.hmd_mask, shader_field(shader, 376), kTextureFlags);

    const auto& hmd_mask = parameter_binding(shader, 0);
    const auto global_field = static_cast<std::uint64_t>(
        ((parameters.hmd_mask != nullptr ? 1U : 0U) - hmd_mask.first_value)
        << hmd_mask.bit_offset) << 32;
    std::uint64_t keys[kRenderShaderProgramKeyCount];
    for (auto& key : keys) {
        key = global_field;
    }
    keys[kPixelKey] = render_shader_parameter_binding_apply(
        global_field,
        parameter_binding(shader, 1),
        parameters.bt709_to_bt2020 ? 1U : 0U);
    keys[kPixelKey] = render_shader_parameter_binding_apply(
        keys[kPixelKey],
        parameter_binding(shader, 2),
        parameters.perceptual_quantizer ? 1U : 0U);
    render_primary_shader_bind(primary_shader(shader), context, keys);
}

}  // namespace rb4
