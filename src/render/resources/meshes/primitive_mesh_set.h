#pragma once

#include <cstddef>

class RndMesh;

namespace rb4 {


struct RenderPrimitiveMeshSet {
    RndMesh** meshes;
    std::size_t mesh_count;
    std::size_t mesh_capacity;
    RndMesh* inline_meshes[2];
};

static_assert(offsetof(RenderPrimitiveMeshSet, meshes) == 0);
static_assert(offsetof(RenderPrimitiveMeshSet, mesh_count) == 8);
static_assert(offsetof(RenderPrimitiveMeshSet, inline_meshes) == 24);
static_assert(sizeof(RenderPrimitiveMeshSet) == 40);

void render_primitive_mesh_set_construct(RenderPrimitiveMeshSet& mesh_set);
void render_primitive_mesh_set_destruct(RenderPrimitiveMeshSet& mesh_set);

}  // namespace rb4
