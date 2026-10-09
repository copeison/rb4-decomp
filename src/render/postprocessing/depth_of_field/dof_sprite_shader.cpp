#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_draw_state.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct DofSpriteShaderDispatch {
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

static_assert(sizeof(DofSpriteShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void dof_sprite_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void dof_sprite_shader_delete(void* shader) {
    dof_sprite_shader_destruct(shader);
    render_release(shader);
}

const void* dof_sprite_shader_source_identifier(void*) {
    return "RndShaderDOFSprite";
}

void* dof_sprite_shader_backend_path(void*) {
    return const_cast<char*>("../../system/data/shaders/DOFSprite.hlsl");
}

void initialize_dof_sprite_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock*,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gBokehTex", "gBokehTexSampler", 1, 4, 12);
    fields[1] = render_shader_backend_add_structured_buffer_input(
        *backend_state, "gSprites", "CSBokehSprite", 0, 0);
}

std::int32_t dof_sprite_shader_mode(void*) {
    return 0;
}

std::int32_t dof_sprite_shader_variant(void*) {
    return 13;
}

DofSpriteShaderDispatch kDofSpriteShaderDispatch{
    dof_sprite_shader_destruct,
    dof_sprite_shader_delete,
    dof_sprite_shader_source_identifier,
    dof_sprite_shader_backend_path,
    initialize_dof_sprite_shader_support_objects,
    dof_sprite_shader_mode,
    dof_sprite_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_returns_true,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6F3330.
void render_dof_sprite_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kDofSpriteShaderDispatch;
    auto* fields = shader_fields(shader);
    fields[0] = -1;
    fields[1] = -1;
}

// Reconstructed from eboot.elf at 0x6F3390. Binds the bokeh texture to the
// pixel stage and the sprite buffer to the vertex stage, which expands each
// sprite through the geometry program.
void render_dof_sprite_shader_draw(
    void* shader,
    RenderContext& context,
    RenderTexture& bokeh,
    RenderComputeBuffer& sprites) {
    constexpr std::uint32_t kVertexStage = 0;
    constexpr std::uint32_t kPixelStage = 4;
    auto* fields = shader_fields(shader);
    render_shader_bind_texture(context, bokeh, kPixelStage, fields[0], 0);
    render_shader_bind_buffer(context, sprites, kVertexStage, fields[1], 0);
    std::uint64_t keys[kRenderShaderProgramKeyCount] = {};
    render_primary_shader_bind(primary_shader(shader), context, keys);
}

}  // namespace rb4
