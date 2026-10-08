#pragma once

#include <cstddef>

#include "orbis_mesh.h"
#include "orbis_vertex_descriptors.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void* render_allocate_named(
    std::size_t size,
    const char* name,
    std::uint32_t alignment);
void mesh_construct(OrbisMesh& mesh, const char* name);
void orbis_mesh_set_format_backend_defaults(
    OrbisMesh& mesh,
    RenderMeshFormat format);
void orbis_position_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_position_mesh_release_vertices(OrbisMesh& mesh);
void orbis_position_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_position_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_color_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_color_mesh_release_vertices(OrbisMesh& mesh);
void orbis_color_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_color_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_color_texture_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_color_texture_mesh_release_vertices(OrbisMesh& mesh);
void orbis_color_texture_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_color_texture_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_unskinned_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_unskinned_mesh_release_vertices(OrbisMesh& mesh);
void orbis_unskinned_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_unskinned_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_skinned_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_skinned_mesh_release_vertices(OrbisMesh& mesh);
void orbis_skinned_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_skinned_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_unskinned_compressed_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_unskinned_compressed_mesh_release_vertices(OrbisMesh& mesh);
void orbis_unskinned_compressed_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_unskinned_compressed_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_skinned_compressed_mesh_release_vertices(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void gnm_buffer_init_as_vertex_buffer(
    OrbisBufferDescriptor& descriptor,
    const void* data,
    OrbisDataFormat format,
    std::uint32_t stride,
    std::uint32_t element_count);
void gnm_buffer_set_resource_memory_type(
    OrbisBufferDescriptor& descriptor,
    OrbisResourceMemoryType memory_type);

}  // namespace rb4
