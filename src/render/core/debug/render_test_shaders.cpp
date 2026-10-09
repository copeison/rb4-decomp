#include "render/resources/shaders/builtin_shader_resources.h"

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

struct RenderTestShaderDispatch {
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

static_assert(sizeof(RenderTestShaderDispatch) == 88);

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

void render_test_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void render_test_shader_delete(void* shader) {
    render_test_shader_destruct(shader);
    render_release(shader);
}

std::int32_t render_test_shader_mode(void*) {
    return 0;
}

std::int32_t render_test_shader_variant(void*) {
    return 13;
}

// Reconstructed from eboot.elf at 0x6455B0.
const void* test_pattern_shader_source_identifier(void*) {
    return "RndShaderTestPattern";
}

// Reconstructed from eboot.elf at 0x645530.
void* test_pattern_shader_backend_path(void*) {
    return const_cast<char*>("../../system/data/shaders/TestPattern.hlsl");
}

// Reconstructed from eboot.elf at 0x645540.
void initialize_test_pattern_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState*) {
    shader_field(shader, 288) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gColor0");
    shader_field(shader, 296) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gColor1");
    shader_field(shader, 304) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector2, "gNumTiles");
    shader_field(shader, 312) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

// Reconstructed from eboot.elf at 0x6427A0.
const void* render_test_simple_shader_source_identifier(void*) {
    return "RndShaderRenderTestSimple";
}

// Reconstructed from eboot.elf at 0x6426B0.
void* render_test_simple_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/RenderTestSimple.hlsl");
}

// Reconstructed from eboot.elf at 0x6426C0. The vertex-color permutation is
// registered in the first registry and the constant-buffer color permutation
// in the pixel-stage registry.
void initialize_render_test_simple_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState*) {
    const Symbol vertex_color("HX_USE_VERTEX_COLOR");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        &parameters->registries[0],
        vertex_color.value());
    const Symbol constant_color("HX_USE_CBUFFER_COLOR");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 1),
        &parameters->registries[4],
        constant_color.value());
    shader_field(shader, 328) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gColor");
    shader_field(shader, 336) = static_cast<std::int64_t>(
        constant_block->next_offset);
}

// The test-pattern dispatch at 0x192F548 uses the destructor and deleting
// destructor at 0x6453F0 and 0x645400.
RenderTestShaderDispatch kTestPatternShaderDispatch{
    render_test_shader_destruct,
    render_test_shader_delete,
    test_pattern_shader_source_identifier,
    test_pattern_shader_backend_path,
    initialize_test_pattern_shader_support_objects,
    render_test_shader_mode,
    render_test_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

// The render-test dispatch at 0x192F418 uses the destructor and deleting
// destructor at 0x642550 and 0x642560.
RenderTestShaderDispatch kRenderTestSimpleShaderDispatch{
    render_test_shader_destruct,
    render_test_shader_delete,
    render_test_simple_shader_source_identifier,
    render_test_simple_shader_backend_path,
    initialize_render_test_simple_shader_support_objects,
    render_test_shader_mode,
    render_test_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x6452E0.
void render_test_pattern_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kTestPatternShaderDispatch;
    shader_field(shader, 288) = -1;
    shader_field(shader, 296) = -1;
    shader_field(shader, 304) = -1;
    shader_field(shader, 312) = 0;
}

// Reconstructed from eboot.elf at 0x642500.
void render_test_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kRenderTestSimpleShaderDispatch;
    for (std::size_t index = 0; index < 2; ++index) {
        parameter_binding(shader, index) = {};
    }
    shader_field(shader, 328) = -1;
    shader_field(shader, 336) = 0;
}

}  // namespace rb4
