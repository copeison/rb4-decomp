#include "render/resources/shaders/fog_deferred_shader.h"

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

constexpr std::size_t kFogDeferredShaderOffset = 3560;

struct FogDeferredShaderDispatch {
    void (*destruct)(FogDeferredShaderResource* shader);
    void (*delete_resource)(FogDeferredShaderResource* shader);
    const void* (*source_identifier)(FogDeferredShaderResource* shader);
    void* (*backend_path)(FogDeferredShaderResource* shader);
    void (*initialize_support_objects)(
        FogDeferredShaderResource* shader,
        void* names,
        RenderShaderParameterRegistrySet* parameters,
        RenderShaderConstantBlock* constant_block,
        RenderShaderBackendState* backend_state);
    std::int32_t (*mode)(FogDeferredShaderResource* shader);
    std::int32_t (*variant)(FogDeferredShaderResource* shader);
    bool (*validate_permutation)(
        void* shader,
        std::uint32_t stage,
        std::uint64_t key);
    void (*bind_fallback)(void* shader, void* context);
    bool (*supports_render_target_slices)(void* shader);
    bool (*uses_geometry_program)(void* shader);
};

struct FogDeferredShaderTail {
    RenderShaderParameterBinding color_space_binding;
    std::uint8_t reserved_308[4];
    std::uint64_t falloff_parameters;
    std::uint64_t sky_texture;
    std::uint64_t linear_depth_texture;
    std::uint64_t function_table_texture;
};

struct FogDeferredShaderLayout {
    std::uint8_t primary_shader[288];
    FogDeferredShaderTail tail;
};

static_assert(sizeof(FogDeferredShaderDispatch) == 88);
static_assert(sizeof(FogDeferredShaderTail) == 56);
static_assert(offsetof(FogDeferredShaderTail, falloff_parameters) == 24);
static_assert(sizeof(FogDeferredShaderLayout) == 344);

RenderPrimaryShaderResource& primary_shader(
    FogDeferredShaderResource& shader) {
    return *reinterpret_cast<RenderPrimaryShaderResource*>(&shader);
}

FogDeferredShaderTail& shader_tail(FogDeferredShaderResource& shader) {
    auto& layout = reinterpret_cast<FogDeferredShaderLayout&>(shader);
    return layout.tail;
}

void fog_deferred_shader_destruct(FogDeferredShaderResource* shader) {
    render_primary_shader_destruct(primary_shader(*shader));
}

void fog_deferred_shader_delete(FogDeferredShaderResource* shader) {
    fog_deferred_shader_destruct(shader);
    render_release(shader);
}

const void* fog_deferred_shader_source_identifier(
    FogDeferredShaderResource*) {
    return "RndShaderFogDeferred";
}

void* fog_deferred_shader_backend_path(FogDeferredShaderResource*) {
    return const_cast<char*>("../../system/data/shaders/FogDeferred.hlsl");
}

void fog_deferred_shader_initialize_support_objects(
    FogDeferredShaderResource* shader,
    void*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto& tail = shader_tail(*shader);

    const Symbol color_space("HX_BT709_TO_BT2020");
    render_shader_parameter_registry_add_ternary(
        &tail.color_space_binding,
        &parameters->registries[4],
        color_space.value());

    tail.falloff_parameters = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector3,
        "gFalloffParams");
    tail.sky_texture = render_shader_backend_add_texture_binding(
        *backend_state,
        "gSkyTex",
        "gSkyTexSampler",
        1,
        4,
        12);
    tail.linear_depth_texture = render_shader_backend_add_texture_binding(
        *backend_state,
        "gLinearDepthMap",
        "gLinearDepthMapSampler",
        1,
        4,
        12);
    tail.function_table_texture = render_shader_backend_add_texture_binding(
        *backend_state,
        "gFunctionTable",
        "gFunctionTableSampler",
        4,
        4,
        12);
}

std::int32_t fog_deferred_shader_mode(FogDeferredShaderResource*) {
    return 0;
}

std::int32_t fog_deferred_shader_variant(FogDeferredShaderResource*) {
    return 13;
}

FogDeferredShaderDispatch kFogDeferredShaderDispatch{
    fog_deferred_shader_destruct,
    fog_deferred_shader_delete,
    fog_deferred_shader_source_identifier,
    fog_deferred_shader_backend_path,
    fog_deferred_shader_initialize_support_objects,
    fog_deferred_shader_mode,
    fog_deferred_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

// Reconstructed from eboot.elf at 0x452CA0.
void fog_deferred_shader_construct(FogDeferredShaderResource& shader) {
    render_primary_shader_construct(primary_shader(shader));
    shader.dispatch = &kFogDeferredShaderDispatch;

    auto& tail = shader_tail(shader);
    tail = {};
    tail.falloff_parameters = UINT64_MAX;
    tail.sky_texture = UINT64_MAX;
    tail.linear_depth_texture = UINT64_MAX;
    tail.function_table_texture = UINT64_MAX;

    render_primary_shader_register(primary_shader(shader));
}

}  // namespace

FogDeferredShaderResource*& render_system_fog_deferred_shader(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<FogDeferredShaderResource**>(
        bytes + kFogDeferredShaderOffset);
}

// Reconstructed from eboot.elf at 0x451C90.
void fog_deferred_shader_create(FogDeferredShaderResource*& shader) {
    auto* created = static_cast<FogDeferredShaderResource*>(
        render_allocate(sizeof(FogDeferredShaderResource)));
    fog_deferred_shader_construct(*created);
    shader = created;
}

// Reconstructed from eboot.elf at 0x451CC0.
void fog_deferred_shader_release(FogDeferredShaderResource*& shader) {
    if (shader != nullptr) {
        auto* dispatch = static_cast<FogDeferredShaderDispatch*>(
            shader->dispatch);
        dispatch->delete_resource(shader);
        shader = nullptr;
    }
}

}  // namespace rb4
