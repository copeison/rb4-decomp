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

struct DownsampleShaderDispatch {
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

static_assert(sizeof(DownsampleShaderDispatch) == 56);

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

void downsample_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void downsample_shader_delete(void* shader) {
    downsample_shader_destruct(shader);
    render_release(shader);
}

const void* downsample_shader_source_identifier(void*) {
    return "RndShaderDownsample";
}

void* downsample_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/Downsample.hlsl");
}

void initialize_downsample_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_COLOR_2X", 0);
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_COLOR_4X", 1);
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_BLOOM_2X", 2);
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_BLOOM_4X", 3);

    auto* pixel_parameters = &parameters->registries[4];
    const Symbol color_space("HX_BT709_TO_BT2020");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        pixel_parameters,
        color_space.value());
    const Symbol downsample_type("HX_DOWNSAMPLE_TYPE");
    render_shader_parameter_registry_add(
        &parameter_binding(shader, 1),
        pixel_parameters,
        downsample_type.value(),
        0,
        4);
    const Symbol value_based_bloom("HX_BLOOM_VALUE_BASED");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 2),
        pixel_parameters,
        value_based_bloom.value());

    shader_field(shader, 352) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gTexelOffset");
    shader_field(shader, 360) = static_cast<std::int64_t>(
        constant_block->next_offset);
    shader_field(shader, 368) =
        render_shader_backend_add_graphics_texture_binding(
            *backend_state, "gTexture", "gTexSampler", 1, 12);
}

std::int32_t downsample_shader_mode(void*) {
    return 0;
}

std::int32_t downsample_shader_variant(void*) {
    return 13;
}

DownsampleShaderDispatch kDownsampleShaderDispatch{
    downsample_shader_destruct,
    downsample_shader_delete,
    downsample_shader_source_identifier,
    downsample_shader_backend_path,
    initialize_downsample_shader_support_objects,
    downsample_shader_mode,
    downsample_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x635FE0.
void render_downsample_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kDownsampleShaderDispatch;
    for (std::size_t index = 0; index < 3; ++index) {
        parameter_binding(shader, index) = {};
    }
    shader_field(shader, 352) = -1;
    shader_field(shader, 360) = 0;
    shader_field(shader, 368) = -1;
}

}  // namespace rb4
