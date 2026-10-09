#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct BloomShaderDispatch {
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

static_assert(sizeof(BloomShaderDispatch) == 88);

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

// Reconstructed from eboot.elf at 0x6346B0.
void bloom_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

// Reconstructed from eboot.elf at 0x6346C0.
void bloom_shader_delete(void* shader) {
    bloom_shader_destruct(shader);
    render_release(shader);
}

// Reconstructed from eboot.elf at 0x634AB0.
const void* bloom_shader_source_identifier(void*) {
    return "RndShaderBloom";
}

// Reconstructed from eboot.elf at 0x634910.
void* bloom_shader_backend_path(void*) {
    return const_cast<char*>("../../system/data/shaders/Bloom.hlsl");
}

// Reconstructed from eboot.elf at 0x634920.
void initialize_bloom_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* pixel_parameters = &parameters->registries[4];
    const Symbol sample_half_size("HX_SAMPLE_HALF_SIZE");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        pixel_parameters,
        sample_half_size.value());
    const Symbol hue_preservation("HX_HUE_PRESERVATION");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 1),
        pixel_parameters,
        hue_preservation.value());

    shader_field(shader, 352) = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcTex", "gSrcTexSampler", 1, 4, 12);
    shader_field(shader, 360) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gHalfSizeBloomTex",
        "gHalfSizeBloomTexSampler",
        1,
        4,
        12);
    shader_field(shader, 368) = render_shader_backend_add_texture_binding(
        *backend_state,
        "gQtrSizeBloomTex",
        "gQtrSizeBloomTexSampler",
        1,
        4,
        12);

    shader_field(shader, 328) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector3, "gBloomParams");
    shader_field(shader, 336) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gOverbrightParams");
    shader_field(shader, 344) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

std::int32_t bloom_shader_mode(void*) {
    return 0;
}

std::int32_t bloom_shader_variant(void*) {
    return 13;
}

BloomShaderDispatch kBloomShaderDispatch{
    bloom_shader_destruct,
    bloom_shader_delete,
    bloom_shader_source_identifier,
    bloom_shader_backend_path,
    initialize_bloom_shader_support_objects,
    bloom_shader_mode,
    bloom_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x634640. The original dispatch table is at
// 0x192EBA0.
void render_bloom_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kBloomShaderDispatch;
    for (std::size_t index = 0; index < 2; ++index) {
        parameter_binding(shader, index) = {};
    }
    shader_field(shader, 328) = -1;
    shader_field(shader, 336) = -1;
    shader_field(shader, 344) = 0;
    shader_field(shader, 352) = -1;
    shader_field(shader, 360) = -1;
    shader_field(shader, 368) = -1;
}

}  // namespace rb4
