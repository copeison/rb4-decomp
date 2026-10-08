#pragma once

#include <cstddef>

#include "orbis_mesh.h"

namespace rb4 {

struct ColorTextureMeshVertex {
    float position[3];
    float color[4];
    float texture_coordinate[2];
};

std::size_t orbis_color_texture_mesh_vertex_count(const OrbisMesh& mesh);
void orbis_color_texture_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count);
void orbis_color_texture_mesh_clear_vertices(OrbisMesh& mesh);
ColorTextureMeshVertex* orbis_color_texture_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index);
ColorTextureMeshVertex* orbis_color_texture_mesh_copy_vertices(
    const OrbisMesh& mesh);
void orbis_color_texture_mesh_finalize_backend(OrbisMesh& mesh);
void orbis_color_texture_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags);

}  // namespace rb4
