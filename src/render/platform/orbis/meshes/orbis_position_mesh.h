#pragma once

#include <cstddef>

#include "render/platform/orbis/meshes/orbis_mesh.h"

namespace rb4 {

struct PositionMeshVertex {
    float position[3];
};

std::size_t orbis_position_mesh_vertex_count(const OrbisMesh& mesh);
void orbis_position_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count);
void orbis_position_mesh_clear_vertices(OrbisMesh& mesh);
PositionMeshVertex* orbis_position_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index);
PositionMeshVertex* orbis_position_mesh_copy_vertices(
    const OrbisMesh& mesh);
void orbis_position_mesh_finalize_backend(OrbisMesh& mesh);
void orbis_position_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags);

}  // namespace rb4
