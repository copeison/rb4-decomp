#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

// One generated-source resource declaration. Textures use kind 0 with a
// texel element type; buffers use kind 1 with either an element type or, for
// structured buffers, a structure name and an invalid element type.
struct RenderShaderBackendBinding {
    std::uint32_t kind;
    std::uint32_t resource_dimension;
    std::uint32_t element_type;
    std::int32_t buffer_kind;
    const void* resource_name;
    const void* sampler_name;
    const void* structure_name;
    std::uint64_t render_target_sliced;
    std::uint64_t resource_index;
    std::uint64_t stage_resource_count;
};

struct RenderShaderBackendBindingArray {
    RenderShaderBackendBinding* begin;
    RenderShaderBackendBinding* end;
    RenderShaderBackendBinding* capacity;
    void* allocator;
};

struct RenderShaderBackendState {
    RenderShaderBackendBindingArray binding_arrays[24];
    std::uint64_t stage_resource_counts[12];
};

static_assert(sizeof(RenderShaderBackendBinding) == 64);
static_assert(offsetof(RenderShaderBackendBinding, structure_name) == 32);
static_assert(offsetof(RenderShaderBackendBinding, resource_index) == 48);
static_assert(sizeof(RenderShaderBackendBindingArray) == 32);
static_assert(sizeof(RenderShaderBackendState) == 864);

void render_shader_backend_state_construct(RenderShaderBackendState& state);
void render_shader_backend_state_destruct(RenderShaderBackendState& state);
void render_shader_backend_state_accumulate_source_hash(
    const RenderShaderBackendState& state,
    std::uint32_t& hash);
std::uint64_t render_shader_backend_add_texture_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* sampler_name,
    std::uint32_t resource_dimension,
    std::uint32_t stage,
    std::uint32_t element_type);
std::uint64_t render_shader_backend_add_graphics_texture_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* sampler_name,
    std::uint32_t resource_dimension,
    std::uint32_t element_type);
std::uint64_t render_shader_backend_add_output_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    std::uint32_t resource_dimension,
    std::uint32_t stage,
    std::uint32_t element_type);
std::uint64_t render_shader_backend_add_buffer_input(
    RenderShaderBackendState& state,
    const char* resource_name,
    std::uint32_t buffer_kind,
    std::uint32_t element_type,
    std::uint32_t stage);
std::uint64_t render_shader_backend_add_buffer_output(
    RenderShaderBackendState& state,
    const char* resource_name,
    std::uint32_t buffer_kind,
    std::uint32_t element_type,
    std::uint32_t stage);
std::uint64_t render_shader_backend_add_structured_buffer_output(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* structure_name,
    std::uint32_t buffer_kind,
    std::uint32_t stage);
std::uint64_t render_shader_backend_add_structured_buffer_input(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* structure_name,
    std::uint32_t buffer_kind,
    std::uint32_t stage);

}  // namespace rb4
