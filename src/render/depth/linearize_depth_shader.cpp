#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct LinearizeDepthShaderDispatch {
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

static_assert(sizeof(LinearizeDepthShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t& depth_texture_binding(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return *reinterpret_cast<std::int64_t*>(bytes + 288);
}

void linearize_depth_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void linearize_depth_shader_delete(void* shader) {
    linearize_depth_shader_destruct(shader);
    render_release(shader);
}

const void* linearize_depth_shader_source_identifier(void*) {
    return "RndShaderLinearizeDepth";
}

void* linearize_depth_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/LinearizeDepth.hlsl");
}

void initialize_linearize_depth_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock*,
    RenderShaderBackendState* backend_state) {
    depth_texture_binding(shader) = render_shader_backend_add_texture_binding(
        *backend_state, "gTexture", "gTexSampler", 1, 4, 12);
}

std::int32_t linearize_depth_shader_mode(void*) {
    return 0;
}

std::int32_t linearize_depth_shader_variant(void*) {
    return 13;
}

LinearizeDepthShaderDispatch kLinearizeDepthShaderDispatch{
    linearize_depth_shader_destruct,
    linearize_depth_shader_delete,
    linearize_depth_shader_source_identifier,
    linearize_depth_shader_backend_path,
    initialize_linearize_depth_shader_support_objects,
    linearize_depth_shader_mode,
    linearize_depth_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x63EF70.
void render_linearize_depth_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kLinearizeDepthShaderDispatch;
    depth_texture_binding(shader) = -1;
}

}  // namespace rb4
