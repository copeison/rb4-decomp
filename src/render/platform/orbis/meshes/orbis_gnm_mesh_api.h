#pragma once

#include <cstddef>

#include "render/platform/orbis/meshes/orbis_mesh.h"
#include "render/platform/orbis/meshes/orbis_mesh_draw.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"

namespace rb4 {

void gnm_buffer_init_as_vertex_buffer(
    OrbisBufferDescriptor& descriptor,
    const void* data,
    OrbisDataFormat format,
    std::uint32_t stride,
    std::uint32_t element_count);
void gnm_buffer_set_resource_memory_type(
    OrbisBufferDescriptor& descriptor,
    OrbisResourceMemoryType memory_type);
enum class OrbisCachePolicy : std::uint32_t {
    kBypass = 2,
};
void orbis_bind_vertex_buffers(
    OrbisRenderCommandContext& context,
    std::uint32_t first_slot,
    std::uint32_t slot_count,
    const OrbisBufferDescriptor* descriptors);
void* orbis_allocate_embedded_data(
    OrbisRenderCommandContext& context,
    std::size_t byte_count,
    std::size_t alignment);
void gnm_draw_command_buffer_set_num_instances(
    OrbisRenderCommandContext& context,
    std::uint32_t instance_count);
void orbis_set_primitive_type(
    OrbisRenderCommandContext& context,
    MeshPrimitiveType primitive_type);
void gnm_draw_command_buffer_set_index_size(
    OrbisRenderCommandContext& context,
    OrbisIndexSize index_size,
    OrbisCachePolicy cache_policy);
void gnmx_prepare_draw(OrbisRenderCommandContext& context);
void gnm_draw_command_buffer_draw_index(
    OrbisRenderCommandContext& context,
    std::uint32_t index_count,
    const void* index_data);
void gnm_draw_command_buffer_draw_index_auto(
    OrbisRenderCommandContext& context,
    std::uint32_t vertex_count);
void gnmx_finish_draw(OrbisRenderCommandContext& context);
}  // namespace rb4
