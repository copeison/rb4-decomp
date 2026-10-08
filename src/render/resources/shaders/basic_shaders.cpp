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

struct BasicShaderDispatch {
    void (*destruct)(void*);
    void (*delete_resource)(void*);
    const void* (*source_identifier)(void*);
    void* (*backend_path)(void*);
    void (*initialize_support_objects)(void*, RenderShaderConstantRegistry*,
        RenderShaderParameterRegistrySet*, RenderShaderConstantBlock*,
        RenderShaderBackendState*);
    std::int32_t (*mode)(void*);
    std::int32_t (*variant)(void*);
};
static_assert(sizeof(BasicShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}
std::uint8_t* bytes(void* shader) { return static_cast<std::uint8_t*>(shader); }
RenderShaderParameterBinding& binding(void* shader, std::size_t index) {
    return *reinterpret_cast<RenderShaderParameterBinding*>(
        bytes(shader) + 288 + index * sizeof(RenderShaderParameterBinding));
}
std::int64_t& field(void* shader, std::size_t offset) {
    return *reinterpret_cast<std::int64_t*>(bytes(shader) + offset);
}
void shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}
void shader_delete(void* shader) { shader_destruct(shader); render_release(shader); }
std::int32_t shader_mode(void*) { return 0; }
std::int32_t shader_variant(void*) { return 13; }

const void* error_identifier(void*) { return "RndShaderError"; }
void* error_path(void*) {
    return const_cast<char*>("../../system/data/shaders/Error.hlsl");
}
void initialize_error(void* shader, RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters, RenderShaderConstantBlock*,
    RenderShaderBackendState*) {
    const Symbol geometry_type("HX_GEO_TYPE");
    render_shader_parameter_registry_add(
        &binding(shader, 0), &parameters->registries[1],
        geometry_type.value(), 0, 2);
    const Symbol shading_mode("HX_SHADING_MODE");
    render_shader_parameter_registry_add(
        &binding(shader, 1), &parameters->registries[4],
        shading_mode.value(), 0, 19);
}

const void* basic_identifier(void*) { return "RndShaderBasic"; }
void* basic_path(void*) {
    return const_cast<char*>("../../system/data/shaders/Basic.hlsl");
}
void initialize_basic(void* shader, RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* pixel = &parameters->registries[4];
    const Symbol shading_mode("HX_SHADING_MODE");
    render_shader_parameter_registry_add(
        &binding(shader, 0), pixel, shading_mode.value(), 0, 19);
    const Symbol texture_mode("HX_TEXTURE_MODE");
    render_shader_parameter_registry_add(
        &binding(shader, 1), pixel, texture_mode.value(), 0, 3);
    const Symbol alpha_cut("HX_ALPHA_CUT");
    render_shader_parameter_registry_add_ternary(
        &binding(shader, 2), pixel, alpha_cut.value());
    const Symbol red_as_alpha("HX_USE_TEX_RED_AS_ALPHA");
    render_shader_parameter_registry_add_ternary(
        &binding(shader, 3), pixel, red_as_alpha.value());
    render_shader_constant_registry_add_definition(
        *constants, "HX_TEXTURE_MODE_NONE", 0);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TEXTURE_MODE_2D", 1);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TEXTURE_MODE_2D_RTSLICED", 2);
    field(shader, 368) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gColor");
    field(shader, 376) = static_cast<std::int64_t>(constant_block->next_offset);
    field(shader, 384) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexture2D", "gTex2DSampler", 1, 4, 12);
    field(shader, 392) = render_shader_backend_add_graphics_texture_binding(
        *backend_state, "gTexture2DRTSliced", "gTex2DRTSlicedSampler", 1, 12);
}

BasicShaderDispatch kErrorDispatch{shader_destruct, shader_delete,
    error_identifier, error_path, initialize_error, shader_mode, shader_variant};
BasicShaderDispatch kBasicDispatch{shader_destruct, shader_delete,
    basic_identifier, basic_path, initialize_basic, shader_mode, shader_variant};

void construct(void* shader, BasicShaderDispatch& dispatch,
    std::size_t binding_count) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &dispatch;
    for (std::size_t index = 0; index < binding_count; ++index) {
        binding(shader, index) = {};
    }
}
}  // namespace

// Reconstructed from eboot.elf at 0x63E650.
void render_error_shader_construct(void* shader) {
    construct(shader, kErrorDispatch, 2);
}

// Reconstructed from eboot.elf at 0x6398D0.
void render_basic_shader_construct(void* shader) {
    construct(shader, kBasicDispatch, 4);
    field(shader, 368) = -1;
    field(shader, 376) = 0;
    field(shader, 384) = -1;
    field(shader, 392) = -1;
}

}  // namespace rb4
