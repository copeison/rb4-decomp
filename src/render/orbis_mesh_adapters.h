#pragma once

#include <cstddef>

#include "orbis_mesh.h"

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

}  // namespace rb4
