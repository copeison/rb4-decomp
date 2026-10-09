#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct CmaaShaderDispatch {
    void (*destruct)(void* shader);
    void (*delete_resource)(void* shader);
    const void* (*source_identifier)(void* shader);
    void* (*backend_path)(void* shader);
    void (*initialize_support_objects)(
        void* shader,
        void* names,
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

static_assert(sizeof(CmaaShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void cmaa_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void cmaa_shader_delete(void* shader) {
    cmaa_shader_destruct(shader);
    render_release(shader);
}

std::int32_t cmaa_shader_mode(void*) {
    return 0;
}

std::int32_t cmaa_compute_shader_variant(void*) {
    return 16;
}

const void* edge_detect_identifier(void*) {
    return "RndCShaderCMAAEdgeDetect";
}

void* edge_detect_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/CMAAEdgeDetect.hlsl");
}

void initialize_edge_detect(
    void* shader,
    void*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gSourceBuffer", "gSourceBufferSampler", 1, 5, 12);
    fields[1] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputEdgesBuffer", 1, 5, 8);
    fields[2] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputColorBuffer", 1, 5, 12);
    fields[3] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::scalar,
        "gEdgeThreshold");
    fields[4] = static_cast<std::int64_t>(constant_block->next_offset);
}

const void* edge_prune_identifier(void*) {
    return "RndCShaderCMAAEdgePrune";
}

void* edge_prune_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/CMAAEdgePrune.hlsl");
}

void initialize_edge_prune(
    void* shader,
    void*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gSourceEdges", "gSourceEdgesSampler", 1, 5, 8);
    fields[1] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputEdges", 1, 5, 12);
    fields[2] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::scalar,
        "gNonDominantEdgeThreshold");
    fields[3] = static_cast<std::int64_t>(constant_block->next_offset);
}

const void* final_process_identifier(void*) {
    return "RndCShaderCMAAFinalProcess";
}

void* final_process_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/CMAAFinalProcess.hlsl");
}

void initialize_final_process(
    void* shader,
    void*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gInputColor", "gInputColorSampler", 1, 5, 12);
    fields[1] = render_shader_backend_add_texture_binding(
        *backend_state, "gInputEdges", "gInputEdgesSampler", 1, 5, 12);
    fields[2] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputBuffer", 1, 5, 12);
    fields[3] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gTargetSizeInv");
    fields[4] = static_cast<std::int64_t>(constant_block->next_offset);
}

const void* shape_fit_identifier(void*) {
    return "RndCShaderCMAAShapeFit";
}

void* shape_fit_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/CMAAShapeFit.hlsl");
}

void initialize_shape_fit(
    void* shader,
    void*,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gInputEdges", "gInputEdgesSampler", 1, 5, 12);
    fields[1] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputColor", 1, 5, 12);
    fields[2] = render_shader_backend_add_output_binding(
        *backend_state, "gOutputEdges", 1, 5, 12);
    fields[3] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gTargetSizeInv");
    fields[4] = static_cast<std::int64_t>(constant_block->next_offset);
}

CmaaShaderDispatch kEdgeDetectDispatch{
    cmaa_shader_destruct,
    cmaa_shader_delete,
    edge_detect_identifier,
    edge_detect_path,
    initialize_edge_detect,
    cmaa_shader_mode,
    cmaa_compute_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

CmaaShaderDispatch kEdgePruneDispatch{
    cmaa_shader_destruct,
    cmaa_shader_delete,
    edge_prune_identifier,
    edge_prune_path,
    initialize_edge_prune,
    cmaa_shader_mode,
    cmaa_compute_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

CmaaShaderDispatch kFinalProcessDispatch{
    cmaa_shader_destruct,
    cmaa_shader_delete,
    final_process_identifier,
    final_process_path,
    initialize_final_process,
    cmaa_shader_mode,
    cmaa_compute_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

CmaaShaderDispatch kShapeFitDispatch{
    cmaa_shader_destruct,
    cmaa_shader_delete,
    shape_fit_identifier,
    shape_fit_path,
    initialize_shape_fit,
    cmaa_shader_mode,
    cmaa_compute_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_supports_render_target_slices,
    render_primary_shader_uses_geometry_program,
};

void construct_cmaa_shader(
    void* shader,
    CmaaShaderDispatch& dispatch,
    std::size_t field_count) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &dispatch;
    auto* fields = shader_fields(shader);
    for (std::size_t index = 0; index + 1 < field_count; ++index) {
        fields[index] = -1;
    }
    fields[field_count - 1] = 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x450490.
void render_cmaa_edge_detect_compute_shader_construct(void* shader) {
    construct_cmaa_shader(shader, kEdgeDetectDispatch, 5);
}

// Reconstructed from eboot.elf at 0x4508B0.
void render_cmaa_edge_prune_compute_shader_construct(void* shader) {
    construct_cmaa_shader(shader, kEdgePruneDispatch, 4);
}

// Reconstructed from eboot.elf at 0x450BF0.
void render_cmaa_final_process_compute_shader_construct(void* shader) {
    construct_cmaa_shader(shader, kFinalProcessDispatch, 5);
}

// Reconstructed from eboot.elf at 0x451030.
void render_cmaa_shape_fit_compute_shader_construct(void* shader) {
    construct_cmaa_shader(shader, kShapeFitDispatch, 5);
}

}  // namespace rb4
