#pragma once

#include <cstddef>

#include "render/platform/orbis/meshes/orbis_mesh.h"

namespace rb4 {

struct UnskinnedMeshVertex {
    float position[3];
    float normal[3];
    float tangent[3];
    float bitangent[3];
    float color[4];
    float texture_coordinate[2];
    float secondary_texture_coordinate[2];
};

std::size_t orbis_unskinned_mesh_vertex_count(const OrbisMesh& mesh);
void orbis_unskinned_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count);
void orbis_unskinned_mesh_clear_vertices(OrbisMesh& mesh);
UnskinnedMeshVertex* orbis_unskinned_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index);
UnskinnedMeshVertex* orbis_unskinned_mesh_copy_vertices(
    const OrbisMesh& mesh);
void orbis_unskinned_mesh_finalize_backend(OrbisMesh& mesh);
void orbis_unskinned_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags);

}  // namespace rb4
