#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct SsaoShaderDispatch {
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

static_assert(sizeof(SsaoShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void ssao_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void ssao_shader_delete(void* shader) {
    ssao_shader_destruct(shader);
    render_release(shader);
}

const void* ssao_shader_source_identifier(void*) {
    return "RndCShaderSSAOGen";
}

void* ssao_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/SSAOGen.hlsl");
}

void initialize_ssao_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gLinearDepthBuffer",
        "gLinearDepthBufferSampler",
        1,
        5,
        12);
    fields[1] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gGBufferNormal",
        "gGBufferNormalSampler",
        1,
        5,
        12);
    fields[2] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gNoiseTex",
        "gNoiseTexSampler",
        1,
        5,
        12);
    fields[3] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gSceneMask",
        "gSceneMaskSampler",
        1,
        5,
        12);
    fields[4] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputBuffer", 1, 5, 12);
    fields[6] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gDimensions");
    fields[7] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gParams");
    fields[5] = static_cast<std::int64_t>(constant_block->next_offset);

    const auto& settings = *render_system_core_state(
        *render_system_instance()).settings;
    render_shader_constant_registry_add_definition(
        *constants,
        "HX_TILE_SIZE",
        static_cast<std::int32_t>(settings.light_tile_size));
}

std::int32_t ssao_shader_mode(void*) {
    return 0;
}

std::int32_t ssao_shader_variant(void*) {
    return 16;
}

SsaoShaderDispatch kSsaoShaderDispatch{
    ssao_shader_destruct,
    ssao_shader_delete,
    ssao_shader_source_identifier,
    ssao_shader_backend_path,
    initialize_ssao_support_objects,
    ssao_shader_mode,
    ssao_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6D7130.
void render_ssao_compute_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kSsaoShaderDispatch;

    auto* fields = shader_fields(shader);
    for (std::size_t index = 0; index < 5; ++index) {
        fields[index] = -1;
    }
    fields[6] = -1;
    fields[7] = -1;
}

}  // namespace rb4
