#pragma once

#include "render/core/meshes/render_mesh.h"

namespace rb4 {

void render_mesh_set_base_dispatch(RenderMesh& mesh);
void render_mesh_update_link_construct(RenderMeshUpdateLink& link);
void render_mesh_update_link_destruct(RenderMeshUpdateLink& link);
void render_mesh_triangle_array_destruct(RenderMeshTriangleArray& triangles);
void render_mesh_triangle_array_clear(RenderMeshTriangleArray& triangles);
void render_mesh_finalize_backend(RenderMesh& mesh);
void render_mesh_release_vertex_storage(RenderMesh& mesh);
void render_mesh_update_backend(RenderMesh& mesh);
void render_delete_mesh_storage(RenderMesh& mesh);
void render_mesh_resize_triangles(
    RenderMesh& mesh,
    std::size_t triangle_count);
void render_mesh_resize_position_vertices(
    RenderMesh& mesh,
    std::size_t vertex_count);
PositionMeshVertex& render_mesh_position_vertex_at(
    RenderMesh& mesh,
    std::size_t index);

}  // namespace rb4
