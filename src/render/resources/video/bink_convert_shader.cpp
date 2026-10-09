#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct BinkConvertShaderDispatch {
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

static_assert(sizeof(BinkConvertShaderDispatch) == 56);

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

// Reconstructed from eboot.elf at 0x5F49E0.
void bink_convert_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

// Reconstructed from eboot.elf at 0x5F49F0.
void bink_convert_shader_delete(void* shader) {
    bink_convert_shader_destruct(shader);
    render_release(shader);
}

// Reconstructed from eboot.elf at 0x5F4E70.
const void* bink_convert_shader_source_identifier(void*) {
    return "RndShaderBinkConvert";
}

// Reconstructed from eboot.elf at 0x5F4C90.
void* bink_convert_shader_backend_path(void*) {
    return const_cast<char*>("../../system/data/shaders/BinkConvert.hlsl");
}

// Reconstructed from eboot.elf at 0x5F4CA0.
void initialize_bink_convert_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    const Symbol alpha_plane("HX_BINK_ALPHA_PLANE");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        &parameters->registries[4],
        alpha_plane.value());

    shader_field(shader, 312) = render_shader_backend_add_texture_binding(
        *backend_state, "gYPlane", "gYPlaneSampler", 1, 4, 12);
    shader_field(shader, 320) = render_shader_backend_add_texture_binding(
        *backend_state, "gCRPlane", "gCRPlaneSampler", 1, 4, 12);
    shader_field(shader, 328) = render_shader_backend_add_texture_binding(
        *backend_state, "gCBPlane", "gCBPlaneSampler", 1, 4, 12);
    shader_field(shader, 336) = render_shader_backend_add_texture_binding(
        *backend_state, "gAPlane", "gAPlaneSampler", 1, 4, 12);

    shader_field(shader, 344) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gYScale");
    shader_field(shader, 352) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gCRScale");
    shader_field(shader, 360) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gCBScale");
    shader_field(shader, 368) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gFullScale");
    shader_field(shader, 376) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gFullOffset");
    shader_field(shader, 384) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

std::int32_t bink_convert_shader_mode(void*) {
    return 0;
}

std::int32_t bink_convert_shader_variant(void*) {
    return 13;
}

BinkConvertShaderDispatch kBinkConvertShaderDispatch{
    bink_convert_shader_destruct,
    bink_convert_shader_delete,
    bink_convert_shader_source_identifier,
    bink_convert_shader_backend_path,
    initialize_bink_convert_shader_support_objects,
    bink_convert_shader_mode,
    bink_convert_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x5F4980. The original dispatch table is at
// 0x192ABA8. The four bytes after the single parameter binding are left
// untouched.
void render_bink_convert_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kBinkConvertShaderDispatch;
    parameter_binding(shader, 0) = {};
    for (std::size_t offset = 312; offset <= 376; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 384) = 0;
}

}  // namespace rb4
