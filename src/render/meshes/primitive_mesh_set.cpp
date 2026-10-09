#include "render/meshes/primitive_mesh_set.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/primitive_mesh_set_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x460640.
void render_primitive_mesh_set_construct(RenderPrimitiveMeshSet& mesh_set) {
    mesh_set.meshes = mesh_set.inline_meshes;
    mesh_set.mesh_count = 2;
    mesh_set.mesh_capacity = 2;
    mesh_set.inline_meshes[0] = render_create_default_box_mesh();
    mesh_set.inline_meshes[1] = render_create_default_cylinder_mesh();
}

// Reconstructed from eboot.elf at 0x460880.
void render_primitive_mesh_set_destruct(RenderPrimitiveMeshSet& mesh_set) {
    for (std::size_t index = 0; index < mesh_set.mesh_count; ++index) {
        if (mesh_set.meshes[index] != nullptr) {
            delete mesh_set.meshes[index];
            mesh_set.meshes[index] = nullptr;
        }
    }
    mesh_set.mesh_count = 0;
}

}  // namespace rb4
