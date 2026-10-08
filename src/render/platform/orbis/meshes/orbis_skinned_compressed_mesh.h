#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/meshes/orbis_mesh.h"

namespace rb4 {

struct SkinnedCompressedMeshVertex {
    float position[3];
    std::int16_t packed_normal[4];
    std::int16_t packed_tangent[4];
    std::int16_t packed_bitangent[4];
    std::uint16_t packed_color[4];
    std::uint16_t packed_texture_coordinate[2];
    std::uint16_t packed_secondary_texture_coordinate[2];
    std::uint16_t packed_bone_weights[4];
    std::uint32_t packed_bone_indices;
};

std::size_t orbis_skinned_compressed_mesh_vertex_count(
    const OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count);
void orbis_skinned_compressed_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count);
void orbis_skinned_compressed_mesh_clear_vertices(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_release_vertices(OrbisMesh& mesh);
SkinnedCompressedMeshVertex* orbis_skinned_compressed_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index);
SkinnedCompressedMeshVertex* orbis_skinned_compressed_mesh_copy_vertices(
    const OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_finalize_backend(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_rebuild_vertex_buffers(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_rebuild_index_buffer(OrbisMesh& mesh);
void orbis_skinned_compressed_mesh_update_backend(
    OrbisMesh& mesh,
    void* update_context,
    MeshUpdateFlags flags);

}  // namespace rb4
