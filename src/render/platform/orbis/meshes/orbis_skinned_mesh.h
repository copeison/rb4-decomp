#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/meshes/orbis_mesh.h"

namespace rb4 {

struct SkinnedMeshVertex {
    float position[3];
    float normal[3];
    float tangent[3];
    float bitangent[3];
    float color[4];
    float texture_coordinate[2];
    float secondary_texture_coordinate[2];
    float bone_weights[4];
    std::uint32_t packed_bone_indices;
};

std::size_t orbis_skinned_mesh_vertex_count(const OrbisMesh& mesh);
void orbis_skinned_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count);
void orbis_skinned_mesh_clear_vertices(OrbisMesh& mesh);
SkinnedMeshVertex* orbis_skinned_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index);
SkinnedMeshVertex* orbis_skinned_mesh_copy_vertices(
    const OrbisMesh& mesh);
void orbis_skinned_mesh_finalize_backend(OrbisMesh& mesh);
void orbis_skinned_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags);

}  // namespace rb4
