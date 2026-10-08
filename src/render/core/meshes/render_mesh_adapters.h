#pragma once

#include "render/core/meshes/render_mesh.h"

namespace rb4 {

void render_mesh_set_base_dispatch(RenderMesh& mesh);
void render_mesh_update_link_construct(RenderMeshUpdateLink& link);
void render_mesh_update_link_destruct(RenderMeshUpdateLink& link);
void render_mesh_triangle_array_destruct(RenderMeshTriangleArray& triangles);
void render_delete_mesh_storage(RenderMesh& mesh);

}  // namespace rb4
