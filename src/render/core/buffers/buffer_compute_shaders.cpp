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

struct BufferShaderDispatch {
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

static_assert(sizeof(BufferShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

RenderShaderParameterBinding& numeric_type_binding(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return *reinterpret_cast<RenderShaderParameterBinding*>(bytes + 288);
}

RenderShaderParameterBinding& texture_type_binding(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return *reinterpret_cast<RenderShaderParameterBinding*>(bytes + 308);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 328);
}

void buffer_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void buffer_shader_delete(void* shader) {
    buffer_shader_destruct(shader);
    render_release(shader);
}

void initialize_buffer_permutations(
    void* shader,
    RenderShaderConstantRegistry& constants,
    RenderShaderParameterRegistrySet& parameters) {
    const Symbol numeric_type("HX_NUMERIC_TYPE");
    render_shader_parameter_registry_add(
        &numeric_type_binding(shader),
        &parameters.registries[5],
        numeric_type.value(),
        0,
        17);

    const Symbol texture_type("HX_TEXTURE_TYPE");
    render_shader_parameter_registry_add(
        &texture_type_binding(shader),
        &parameters.registries[5],
        texture_type.value(),
        UINT32_MAX,
        8);

    render_shader_constant_registry_add_definition(
        constants, "HX_NUMERIC_TYPE_UINT", 5);
    render_shader_constant_registry_add_definition(
        constants, "HX_NUMERIC_TYPE_FLOAT4", 12);
    render_shader_constant_registry_add_definition(
        constants, "HX_TEXTURE_TYPE_INVALID", -1);
    render_shader_constant_registry_add_definition(
        constants, "HX_TEXTURE_TYPE_1D", 0);
    render_shader_constant_registry_add_definition(
        constants, "HX_TEXTURE_TYPE_2D", 1);
}

const void* clear_buffer_source_identifier(void*) {
    return "RndCShaderClearBuffer";
}

void* clear_buffer_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/ClearBuffer.hlsl");
}

void initialize_clear_buffer_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    initialize_buffer_permutations(shader, *constants, *parameters);

    auto* fields = shader_fields(shader);
    fields[0] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gClearColor");
    fields[1] = static_cast<std::int64_t>(constant_block->next_offset);
    fields[2] = render_shader_backend_add_buffer_output(
        *backend_state, "gUintBuffer", 0, 5, 5);
    fields[3] = render_shader_backend_add_output_binding(
        *backend_state, "gUintTex1D", 0, 5, 5);
    fields[4] = render_shader_backend_add_output_binding(
        *backend_state, "gUintTex2D", 1, 5, 5);
    fields[5] = render_shader_backend_add_buffer_output(
        *backend_state, "gFloat4Buffer", 0, 12, 5);
    fields[6] = render_shader_backend_add_output_binding(
        *backend_state, "gFloat4Tex1D", 0, 5, 12);
    fields[7] = render_shader_backend_add_output_binding(
        *backend_state, "gFloat4Tex2D", 1, 5, 12);
}

const void* copy_buffer_source_identifier(void*) {
    return "RndCShaderCopyBuffer";
}

void* copy_buffer_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/CopyBuffer.hlsl");
}

void initialize_copy_buffer_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock*,
    RenderShaderBackendState* backend_state) {
    initialize_buffer_permutations(shader, *constants, *parameters);

    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_buffer_input(
        *backend_state, "gSrcUintBuffer", 0, 5, 5);
    fields[1] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcUintTex1D", "gSrcUintTex1DSampler", 0, 5, 5);
    fields[2] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcUintTex2D", "gSrcUintTex2DSampler", 1, 5, 5);
    fields[3] = render_shader_backend_add_buffer_input(
        *backend_state, "gSrcFloat4Buffer", 0, 12, 5);
    fields[4] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gSrcFloat4Tex1D",
        "gSrcFloat4Tex1DSampler",
        0,
        5,
        12);
    fields[5] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gSrcFloat4Tex2D",
        "gSrcFloat4Tex2DSampler",
        1,
        5,
        12);
    fields[6] = render_shader_backend_add_buffer_output(
        *backend_state, "gDestUintBuffer", 0, 5, 5);
    fields[7] = render_shader_backend_add_output_binding(
        *backend_state, "gDestUintTex1D", 0, 5, 5);
    fields[8] = render_shader_backend_add_output_binding(
        *backend_state, "gDestUintTex2D", 1, 5, 5);
    fields[9] = render_shader_backend_add_buffer_output(
        *backend_state, "gDestFloat4Buffer", 0, 12, 5);
    fields[10] = render_shader_backend_add_output_binding(
        *backend_state, "gDestFloat4Tex1D", 0, 5, 12);
    fields[11] = render_shader_backend_add_output_binding(
        *backend_state, "gDestFloat4Tex2D", 1, 5, 12);
}

std::int32_t buffer_shader_mode(void*) {
    return 0;
}

std::int32_t buffer_shader_variant(void*) {
    return 16;
}

BufferShaderDispatch kClearBufferShaderDispatch{
    buffer_shader_destruct,
    buffer_shader_delete,
    clear_buffer_source_identifier,
    clear_buffer_backend_path,
    initialize_clear_buffer_support_objects,
    buffer_shader_mode,
    buffer_shader_variant,
};

BufferShaderDispatch kCopyBufferShaderDispatch{
    buffer_shader_destruct,
    buffer_shader_delete,
    copy_buffer_source_identifier,
    copy_buffer_backend_path,
    initialize_copy_buffer_support_objects,
    buffer_shader_mode,
    buffer_shader_variant,
};

void construct_buffer_shader(
    void* shader,
    BufferShaderDispatch& dispatch) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &dispatch;
    numeric_type_binding(shader) = {};
    texture_type_binding(shader) = {};
}

}  // namespace

// Reconstructed from eboot.elf at 0x637210.
void render_clear_buffer_compute_shader_construct(void* shader) {
    construct_buffer_shader(shader, kClearBufferShaderDispatch);
    auto* fields = shader_fields(shader);
    fields[0] = -1;
    fields[1] = 0;
    for (std::size_t index = 2; index < 8; ++index) {
        fields[index] = -1;
    }
}

// Reconstructed from eboot.elf at 0x6F3550.
void render_copy_buffer_compute_shader_construct(void* shader) {
    construct_buffer_shader(shader, kCopyBufferShaderDispatch);
    auto* fields = shader_fields(shader);
    for (std::size_t index = 0; index < 12; ++index) {
        fields[index] = -1;
    }
}

}  // namespace rb4
