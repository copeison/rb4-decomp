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

struct RenderTestComputeShaderDispatch {
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

static_assert(sizeof(RenderTestComputeShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::uint8_t* shader_bytes(void* shader) {
    return static_cast<std::uint8_t*>(shader);
}

RenderShaderParameterBinding& output_type_binding(void* shader) {
    return *reinterpret_cast<RenderShaderParameterBinding*>(
        shader_bytes(shader) + 288);
}

std::int64_t& shader_field(void* shader, std::size_t offset) {
    return *reinterpret_cast<std::int64_t*>(shader_bytes(shader) + offset);
}

void render_test_compute_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void render_test_compute_shader_delete(void* shader) {
    render_test_compute_shader_destruct(shader);
    render_release(shader);
}

const void* render_test_compute_shader_source_identifier(void*) {
    return "RndCShaderRenderTestCompute";
}

void* render_test_compute_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/RenderTestCompute.hlsl");
}

void initialize_render_test_compute_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    const Symbol write_to_buffer("HX_WRITE_TO_BUFFER");
    render_shader_parameter_registry_add_ternary(
        &output_type_binding(shader),
        &parameters->registries[5],
        write_to_buffer.value());
    shader_field(shader, 312) = render_shader_backend_add_output_binding(
        *backend_state, "gDstTexture", 1, 5, 12);
    shader_field(shader, 320) = render_shader_backend_add_buffer_output(
        *backend_state, "gDstBuffer", 0, 12, 5);
    shader_field(shader, 328) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gDimensions");
    shader_field(shader, 336) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gAnimParams");
    shader_field(shader, 344) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

std::int32_t render_test_compute_shader_mode(void*) {
    return 0;
}

std::int32_t render_test_compute_shader_variant(void*) {
    return 16;
}

RenderTestComputeShaderDispatch kRenderTestComputeShaderDispatch{
    render_test_compute_shader_destruct,
    render_test_compute_shader_delete,
    render_test_compute_shader_source_identifier,
    render_test_compute_shader_backend_path,
    initialize_render_test_compute_shader_support_objects,
    render_test_compute_shader_mode,
    render_test_compute_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6F3E00.
void render_test_compute_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kRenderTestComputeShaderDispatch;
    output_type_binding(shader) = {};
    for (std::size_t offset = 312; offset <= 336; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 344) = 0;
}

}  // namespace rb4
