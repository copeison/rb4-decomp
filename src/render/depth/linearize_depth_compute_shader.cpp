#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"
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

struct LinearizeDepthShaderTail {
    RenderShaderParameterBinding orthographic_binding;
    std::uint8_t reserved_308[4];
    std::int64_t depth_texture;
    std::int64_t linear_depth_output;
    std::int64_t dimensions;
    std::int64_t constant_block_size;
};

struct LinearizeDepthShaderLayout {
    std::uint8_t primary_shader[288];
    LinearizeDepthShaderTail tail;
};

static_assert(sizeof(LinearizeDepthShaderDispatch) == 88);
static_assert(sizeof(LinearizeDepthShaderTail) == 56);
static_assert(sizeof(LinearizeDepthShaderLayout) == 344);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

LinearizeDepthShaderTail& shader_tail(void* shader) {
    auto& layout = *static_cast<LinearizeDepthShaderLayout*>(shader);
    return layout.tail;
}

void linearize_depth_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void linearize_depth_shader_delete(void* shader) {
    linearize_depth_shader_destruct(shader);
    MemFree(shader);
}

const void* linearize_depth_shader_source_identifier(void*) {
    return "RndCShaderLinearizeDepth";
}

void* linearize_depth_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/LinearizeDepthCompute.hlsl");
}

void initialize_linearize_depth_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto& tail = shader_tail(shader);

    render_shader_constant_registry_add_definition(
        *constants, "HX_TILE_SIZE_X", 2);
    render_shader_constant_registry_add_definition(
        *constants, "HX_TILE_SIZE_Y", 2);

    const Symbol is_orthographic("HX_IS_ORTHO");
    render_shader_parameter_registry_add_ternary(
        &tail.orthographic_binding,
        &parameters->registries[5],
        is_orthographic.Str());

    tail.depth_texture = render_shader_backend_add_texture_binding(
        *backend_state, "gDepth", "", 1, 5, 9);
    tail.linear_depth_output = render_shader_backend_add_output_binding(
        *backend_state, "gLinearDepth", 1, 5, 9);
    tail.dimensions = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gDimensions");
    tail.constant_block_size = static_cast<std::int64_t>(
        constant_block->next_offset);
}

std::int32_t linearize_depth_shader_mode(void*) {
    return 0;
}

std::int32_t linearize_depth_shader_variant(void*) {
    return 16;
}

LinearizeDepthShaderDispatch kLinearizeDepthShaderDispatch{
    linearize_depth_shader_destruct,
    linearize_depth_shader_delete,
    linearize_depth_shader_source_identifier,
    linearize_depth_shader_backend_path,
    initialize_linearize_depth_support_objects,
    linearize_depth_shader_mode,
    linearize_depth_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x637C10.
void render_linearize_depth_compute_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kLinearizeDepthShaderDispatch;

    auto& tail = shader_tail(shader);
    tail = {};
    tail.depth_texture = -1;
    tail.linear_depth_output = -1;
    tail.dimensions = -1;
    tail.constant_block_size = 0;
}

}  // namespace rb4
