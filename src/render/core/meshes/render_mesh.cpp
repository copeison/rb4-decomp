#include "render/core/meshes/render_mesh.h"

#include <limits>

#include "render/core/meshes/render_mesh_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x5C2700.
void render_mesh_construct(RenderMesh& mesh, const char* name) {
    render_mesh_update_link_construct(mesh.update_link);
    render_mesh_set_base_dispatch(mesh);

    mesh.triangles = {};
    mesh.vertex_count = 0;
    mesh.triangle_count = 0;
    mesh.geometry_frame = -1;
    mesh.vertices_resident = false;
    mesh.triangles_resident = false;
    mesh.vertex_usage_flags = 0;
    mesh.triangle_usage_flags = 0;
    for (auto& value : mesh.metadata_sentinel) {
        value = std::numeric_limits<std::uint32_t>::max();
    }
    mesh.pending_update_flags = 0;
    mesh.last_used_frame = std::numeric_limits<std::uint64_t>::max();
    mesh.name = name;
}

// Reconstructed from eboot.elf at 0x5C27B0.
void render_mesh_destruct(RenderMesh& mesh) {
    render_mesh_set_base_dispatch(mesh);
    render_mesh_triangle_array_destruct(mesh.triangles);
    render_mesh_update_link_destruct(mesh.update_link);
}

// Reconstructed from eboot.elf at 0x5C2870.
void render_mesh_delete(RenderMesh& mesh) {
    render_mesh_destruct(mesh);
    render_delete_mesh_storage(mesh);
}

}  // namespace rb4
