#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct FxaaShaderDispatch {
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

static_assert(sizeof(FxaaShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void fxaa_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void fxaa_shader_delete(void* shader) {
    fxaa_shader_destruct(shader);
    render_release(shader);
}

const void* fxaa_shader_source_identifier(void*) {
    return "RndShaderFXAA";
}

void* fxaa_shader_backend_path(void*) {
    return const_cast<char*>("../../system/data/shaders/FXAA.hlsl");
}

void initialize_fxaa_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[2] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcTex", "gSrcTexSampler", 1, 4, 12);
    fields[0] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gTargetDimensionsRcp");
    fields[1] = static_cast<std::int64_t>(constant_block->next_offset);
}

std::int32_t fxaa_shader_mode(void*) {
    return 0;
}

std::int32_t fxaa_shader_variant(void*) {
    return 13;
}

FxaaShaderDispatch kFxaaShaderDispatch{
    fxaa_shader_destruct,
    fxaa_shader_delete,
    fxaa_shader_source_identifier,
    fxaa_shader_backend_path,
    initialize_fxaa_shader_support_objects,
    fxaa_shader_mode,
    fxaa_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6364F0.
void render_fxaa_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kFxaaShaderDispatch;
    auto* fields = shader_fields(shader);
    fields[0] = -1;
    fields[1] = 0;
    fields[2] = -1;
}

}  // namespace rb4
