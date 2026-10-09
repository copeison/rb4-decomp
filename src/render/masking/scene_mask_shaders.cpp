#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "os/memory/MemMgr.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_draw_state.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct SceneMaskShaderDispatch {
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

static_assert(sizeof(SceneMaskShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void scene_mask_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void scene_mask_shader_delete(void* shader) {
    scene_mask_shader_destruct(shader);
    MemFree(shader);
}

std::int32_t scene_mask_shader_mode(void*) {
    return 0;
}

const void* refine_scene_mask_source_identifier(void*) {
    return "RndShaderRefineSceneMask";
}

void* refine_scene_mask_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/RefineSceneMask.hlsl");
}

void initialize_refine_scene_mask_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock*,
    RenderShaderBackendState* backend_state) {
    shader_fields(shader)[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gUnrefinedMask", "", 1, 4, 12);
}

std::int32_t refine_scene_mask_variant(void*) {
    return 13;
}

const void* stencil_scene_mask_source_identifier(void*) {
    return "RndShaderStencilSceneMask";
}

void* stencil_scene_mask_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/StencilSceneMask.hlsl");
}

void initialize_stencil_scene_mask_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    static const RenderSettings default_settings{};
    auto* system = render_system_instance();
    auto* settings = system == nullptr ? nullptr : render_system_settings(*system);
    const auto tile_size = static_cast<std::int32_t>(
        (settings == nullptr ? default_settings : *settings).light_tile_size);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TILE_SIZE", tile_size);

    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gUnrefinedMask", "", 1, 0, 12);
    fields[1] = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gTileCounts");
    fields[2] = static_cast<std::int64_t>(constant_block->next_offset);
}

std::int32_t stencil_scene_mask_variant(void*) {
    return 1;
}

SceneMaskShaderDispatch kRefineSceneMaskDispatch{
    scene_mask_shader_destruct,
    scene_mask_shader_delete,
    refine_scene_mask_source_identifier,
    refine_scene_mask_backend_path,
    initialize_refine_scene_mask_support_objects,
    scene_mask_shader_mode,
    refine_scene_mask_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

SceneMaskShaderDispatch kStencilSceneMaskDispatch{
    scene_mask_shader_destruct,
    scene_mask_shader_delete,
    stencil_scene_mask_source_identifier,
    stencil_scene_mask_backend_path,
    initialize_stencil_scene_mask_support_objects,
    scene_mask_shader_mode,
    stencil_scene_mask_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x642360.
void render_refine_scene_mask_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kRefineSceneMaskDispatch;
    shader_fields(shader)[0] = -1;
}

// Reconstructed from eboot.elf at 0x644FC0.
void render_stencil_scene_mask_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kStencilSceneMaskDispatch;
    auto* fields = shader_fields(shader);
    fields[0] = -1;
    fields[1] = -1;
    fields[2] = 0;
}

// Reconstructed from eboot.elf at 0x6423C0.
void render_refine_scene_mask_shader_draw(
    void* shader,
    RndContext& context,
    RndTextureBase& unrefined_mask) {
    render_shader_draw_with_pixel_texture(shader, context, unrefined_mask, 288);
}

}  // namespace rb4
