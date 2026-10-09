#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct SignedDistanceShaderDispatch {
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

static_assert(sizeof(SignedDistanceShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void signed_distance_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void signed_distance_shader_delete(void* shader) {
    signed_distance_shader_destruct(shader);
    MemFree(shader);
}

const void* signed_distance_source_identifier(void*) {
    return "RndCShaderSignedDistance";
}

void* signed_distance_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/SignedDistance.hlsl");
}

void initialize_signed_distance_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcBuffer", "", 1, 5, 12);
    fields[1] = render_shader_backend_add_texture_binding(
        *backend_state, "gClassificationBuffer", "", 1, 5, 12);
    fields[2] = render_shader_backend_add_output_binding(
        *backend_state, "gDstBuffer", 1, 5, 12);
    fields[4] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gDimensions");
    fields[5] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector3,
        "gTileParams");
    fields[6] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gDistanceParams");
    fields[3] = static_cast<std::int64_t>(constant_block->next_offset);
}

const void* classify_source_identifier(void*) {
    return "RndCShaderSignedDistanceClassify";
}

void* classify_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/SignedDistanceClassify.hlsl");
}

void initialize_classify_support_objects(
    void* shader,
    RenderShaderConstantRegistry*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gSrcBuffer", "", 1, 5, 12);
    fields[1] = render_shader_backend_add_output_binding(
        *backend_state, "gDstBuffer", 1, 5, 12);
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

std::int32_t signed_distance_shader_mode(void*) {
    return 0;
}

std::int32_t signed_distance_shader_variant(void*) {
    return 16;
}

SignedDistanceShaderDispatch kSignedDistanceShaderDispatch{
    signed_distance_shader_destruct,
    signed_distance_shader_delete,
    signed_distance_source_identifier,
    signed_distance_backend_path,
    initialize_signed_distance_support_objects,
    signed_distance_shader_mode,
    signed_distance_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

SignedDistanceShaderDispatch kClassifyShaderDispatch{
    signed_distance_shader_destruct,
    signed_distance_shader_delete,
    classify_source_identifier,
    classify_backend_path,
    initialize_classify_support_objects,
    signed_distance_shader_mode,
    signed_distance_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

void construct_signed_distance_shader(
    void* shader,
    SignedDistanceShaderDispatch& dispatch,
    std::size_t field_count) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &dispatch;
    auto* fields = shader_fields(shader);
    for (std::size_t index = 0; index < field_count; ++index) {
        fields[index] = -1;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x62E660.
void render_signed_distance_compute_shader_construct(void* shader) {
    construct_signed_distance_shader(shader, kSignedDistanceShaderDispatch, 7);
}

// Reconstructed from eboot.elf at 0x62EAF0.
void render_signed_distance_classify_compute_shader_construct(void* shader) {
    construct_signed_distance_shader(shader, kClassifyShaderDispatch, 6);
}

}  // namespace rb4
