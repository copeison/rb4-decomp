#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct DofDiscBlurShaderDispatch {
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

static_assert(sizeof(DofDiscBlurShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void dof_disc_blur_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void dof_disc_blur_shader_delete(void* shader) {
    dof_disc_blur_shader_destruct(shader);
    render_release(shader);
}

const void* dof_disc_blur_source_identifier(void*) {
    return "RndCShaderDOFDiscBlur";
}

void* dof_disc_blur_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/DOFDiscBlur.hlsl");
}

void initialize_dof_disc_blur_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gSceneTex", "gSceneTexSampler", 1, 5, 12);
    fields[1] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gNormalizedDepthTex",
        "gNormalizedDepthTexSampler",
        1,
        5,
        12);
    fields[2] = render_shader_backend_add_texture_binding(
        *backend_state, "gSceneMask", "gSceneMaskSampler", 1, 5, 12);
    fields[3] = render_shader_backend_add_texture_binding(
        *backend_state,
        "gFunctionTable",
        "gFunctionTableSampler",
        4,
        5,
        12);
    fields[4] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputSceneTex", 1, 5, 12);
    fields[5] = render_shader_backend_add_structured_buffer_output(
        *backend_state, "gSprites", "CSBokehSprite", 1, 5);
    fields[6] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector4,
        "gTextureSize");
    fields[7] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector3,
        "gFalloffParams");
    fields[8] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gBlurParams");
    fields[9] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::scalar,
        "gOverbrightLuminance");
    fields[10] = static_cast<std::int64_t>(constant_block->next_offset);

    auto* system = render_system_instance();
    auto* settings = system == nullptr ? nullptr : render_system_settings(*system);
    const auto tile_size = settings == nullptr
        ? std::int32_t{32}
        : static_cast<std::int32_t>(settings->light_tile_size);
    render_shader_constant_registry_add_definition(
        *constants, "HX_MAX_RADIUS", tile_size / 2);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TILE_SIZE", tile_size);
}

std::int32_t dof_disc_blur_shader_mode(void*) {
    return 0;
}

std::int32_t dof_disc_blur_shader_variant(void*) {
    return 16;
}

DofDiscBlurShaderDispatch kDofDiscBlurShaderDispatch{
    dof_disc_blur_shader_destruct,
    dof_disc_blur_shader_delete,
    dof_disc_blur_source_identifier,
    dof_disc_blur_backend_path,
    initialize_dof_disc_blur_support_objects,
    dof_disc_blur_shader_mode,
    dof_disc_blur_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6F2B40.
void render_dof_disc_blur_compute_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kDofDiscBlurShaderDispatch;
    auto* fields = shader_fields(shader);
    for (std::size_t index = 0; index < 10; ++index) {
        fields[index] = -1;
    }
    fields[10] = 0;
}

}  // namespace rb4
