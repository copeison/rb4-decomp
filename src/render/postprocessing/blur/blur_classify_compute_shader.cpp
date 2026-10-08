#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct BlurClassifyShaderDispatch {
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

static_assert(sizeof(BlurClassifyShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void blur_classify_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void blur_classify_shader_delete(void* shader) {
    blur_classify_shader_destruct(shader);
    render_release(shader);
}

const void* blur_classify_source_identifier(void*) {
    return "RndCShaderBlurClassify";
}

void* blur_classify_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/BlurClassify.hlsl");
}

void initialize_blur_classify_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcBuffer", "", 1, 5, 12);
    fields[1] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcClassificationBuffer", "", 1, 5, 12);
    fields[2] = render_shader_backend_add_output_binding(
        *backend_state, "gDstClassificationBuffer", 1, 5, 12);
    fields[4] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gDimensions");
    fields[5] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector3,
        "gTileParams");
    fields[3] = static_cast<std::int64_t>(constant_block->next_offset);
}

std::int32_t blur_classify_shader_mode(void*) {
    return 0;
}

std::int32_t blur_classify_shader_variant(void*) {
    return 16;
}

BlurClassifyShaderDispatch kBlurClassifyShaderDispatch{
    blur_classify_shader_destruct,
    blur_classify_shader_delete,
    blur_classify_source_identifier,
    blur_classify_backend_path,
    initialize_blur_classify_support_objects,
    blur_classify_shader_mode,
    blur_classify_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x62E1C0.
void render_blur_classify_compute_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kBlurClassifyShaderDispatch;
    auto* fields = shader_fields(shader);
    for (std::size_t index = 0; index < 6; ++index) {
        fields[index] = -1;
    }
}

}  // namespace rb4
