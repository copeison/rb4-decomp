#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_draw_state.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct DisplayShaderDispatch {
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

static_assert(sizeof(DisplayShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::uint8_t* shader_bytes(void* shader) {
    return static_cast<std::uint8_t*>(shader);
}

std::int64_t& shader_field(void* shader, std::size_t offset) {
    return *reinterpret_cast<std::int64_t*>(shader_bytes(shader) + offset);
}

RenderShaderParameterBinding& parameter_binding(
    void* shader,
    std::size_t index) {
    return *reinterpret_cast<RenderShaderParameterBinding*>(
        shader_bytes(shader) + 288 + index * sizeof(RenderShaderParameterBinding));
}

void display_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void display_shader_delete(void* shader) {
    display_shader_destruct(shader);
    MemFree(shader);
}

std::int32_t display_shader_mode(void*) {
    return 0;
}

std::int32_t display_shader_variant(void*) {
    return 13;
}

const void* shading_mode_identifier(void*) {
    return "RndShaderDisplayShadingMode";
}

void* shading_mode_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/DisplayShadingMode.hlsl");
}

void initialize_shading_mode(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    shader_field(shader, 288) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector3, "gMaxOverdraw");
    shader_field(shader, 296) = static_cast<std::int64_t>(
        constant_block->next_offset);
    shader_field(shader, 304) = render_shader_backend_add_texture_binding(
        *backend_state, "gSceneTex", "gSceneTexSampler", 1, 4, 12);
}

const void* sphere_map_identifier(void*) {
    return "RndShaderDisplaySphereMap";
}

void* sphere_map_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/DisplaySphereMap.hlsl");
}

void initialize_sphere_map(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock*,
    RenderShaderBackendState* backend_state) {
    shader_field(shader, 288) = render_shader_backend_add_texture_binding(
        *backend_state, "gSphereMap", "gSphereMapSampler", 1, 4, 12);
}

const void* texture_cube_identifier(void*) {
    return "RndShaderDisplayTextureCube";
}

void* texture_cube_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/DisplayTextureCube.hlsl");
}

void initialize_texture_cube(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* pixel_parameters = &parameters->registries[4];
    const Symbol use_mip_level("HX_USE_MIP_LEVEL");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        pixel_parameters,
        use_mip_level.Str());
    const Symbol blend_textures("HX_BLEND_TEXTURES");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 1),
        pixel_parameters,
        blend_textures.Str());
    const Symbol use_texture_array("HX_USE_TEXARRAY");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 2),
        pixel_parameters,
        use_texture_array.Str());

    shader_field(shader, 352) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gColor");
    shader_field(shader, 360) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gMipLevel");
    shader_field(shader, 368) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::scalar, "gBlendAmount");
    shader_field(shader, 376) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gArrayIndices");
    shader_field(shader, 384) = static_cast<std::int64_t>(
        constant_block->next_offset);
    shader_field(shader, 392) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexture0", "gTexSampler0", 3, 4, 12);
    shader_field(shader, 400) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexture1", "gTexSampler1", 3, 4, 12);
    shader_field(shader, 408) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexArray", "gTexArraySampler", 7, 4, 12);
}

DisplayShaderDispatch kShadingModeDispatch{
    display_shader_destruct, display_shader_delete, shading_mode_identifier,
    shading_mode_path, initialize_shading_mode, display_shader_mode,
    display_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

DisplayShaderDispatch kSphereMapDispatch{
    display_shader_destruct, display_shader_delete, sphere_map_identifier,
    sphere_map_path, initialize_sphere_map, display_shader_mode,
    display_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

DisplayShaderDispatch kTextureCubeDispatch{
    display_shader_destruct, display_shader_delete, texture_cube_identifier,
    texture_cube_path, initialize_texture_cube, display_shader_mode,
    display_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

void construct_shader(void* shader, DisplayShaderDispatch& dispatch) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &dispatch;
}

}  // namespace

// Reconstructed from eboot.elf at 0x63DC90.
void render_display_shading_mode_shader_construct(void* shader) {
    construct_shader(shader, kShadingModeDispatch);
    shader_field(shader, 288) = -1;
    shader_field(shader, 296) = 0;
    shader_field(shader, 304) = -1;
}

// Reconstructed from eboot.elf at 0x6F4270.
void render_display_sphere_map_shader_construct(void* shader) {
    construct_shader(shader, kSphereMapDispatch);
    shader_field(shader, 288) = -1;
}

// Reconstructed from eboot.elf at 0x63DFD0.
void render_display_texture_cube_shader_construct(void* shader) {
    construct_shader(shader, kTextureCubeDispatch);
    for (std::size_t index = 0; index < 3; ++index) {
        parameter_binding(shader, index) = {};
    }
    for (std::size_t offset = 352; offset <= 376; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 384) = 0;
    shader_field(shader, 392) = -1;
    shader_field(shader, 400) = -1;
    shader_field(shader, 408) = -1;
}

// Reconstructed from eboot.elf at 0x6F42D0.
void render_display_sphere_map_shader_draw(
    void* shader,
    RndContext& context,
    RndTextureBase& texture) {
    render_shader_draw_with_pixel_texture(shader, context, texture, 288);
}

}  // namespace rb4
