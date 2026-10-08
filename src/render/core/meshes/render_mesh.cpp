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
    mesh.pending_update_flags.store(0, std::memory_order_relaxed);
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

// Reconstructed from eboot.elf at 0x5C2930.
void render_mesh_set_vertices_resident(RenderMesh& mesh, bool resident) {
    mesh.vertices_resident = resident;
}

// Reconstructed from eboot.elf at 0x5C2940.
void render_mesh_set_vertex_usage_flags(
    RenderMesh& mesh,
    std::uint32_t flags) {
    mesh.vertex_usage_flags = flags;
}

// Reconstructed from eboot.elf at 0x5C2950.
void render_mesh_set_triangle_usage_flags(
    RenderMesh& mesh,
    std::uint32_t flags) {
    mesh.triangle_usage_flags = flags;
}

// Reconstructed from eboot.elf at 0x5C2960.
bool render_mesh_requires_vertex_storage(const RenderMesh& mesh) {
    return mesh.vertices_resident || mesh.triangles_resident ||
           (mesh.vertex_usage_flags & 5U) != 0;
}

// Reconstructed from eboot.elf at 0x5C2980.
bool render_mesh_requires_triangle_storage(const RenderMesh& mesh) {
    return mesh.vertices_resident || mesh.triangles_resident ||
           (mesh.triangle_usage_flags & 5U) != 0;
}

// Reconstructed from eboot.elf at 0x5C29A0.
void render_mesh_finalize(RenderMesh& mesh) {
    mesh.triangle_count = static_cast<std::size_t>(
        mesh.triangles.end - mesh.triangles.begin);
    render_mesh_finalize_backend(mesh);

    if (mesh.vertices_resident || mesh.triangles_resident) {
        return;
    }
    if ((mesh.vertex_usage_flags & 5U) == 0) {
        render_mesh_release_vertex_storage(mesh);
        if (mesh.vertices_resident) {
            return;
        }
    }
    if (!mesh.triangles_resident &&
        (mesh.triangle_usage_flags & 5U) == 0) {
        render_mesh_triangle_array_clear(mesh.triangles);
    }
}

// Reconstructed from eboot.elf at 0x5C2DF0.
void render_mesh_apply_updates(
    RenderMesh& mesh,
    void* update_context,
    std::uint32_t flags) {
    (void)update_context;

    mesh.last_used_frame = current_render_epoch();
    if ((flags & 2U) != 0) {
        mesh.triangle_count = static_cast<std::size_t>(
            mesh.triangles.end - mesh.triangles.begin);
    }
    render_mesh_update_backend(mesh);
}

// Reconstructed from eboot.elf at 0x5C2E40.
void render_mesh_process_pending_updates(RenderMesh& mesh) {
    const auto flags =
        mesh.pending_update_flags.load(std::memory_order_relaxed);
    if (flags == 0) {
        return;
    }

    render_mesh_apply_updates(mesh, nullptr, flags);
    mesh.pending_update_flags.exchange(0);
}

// Reconstructed from eboot.elf at 0x5C2EA0.
void render_mesh_process_pending_updates_secondary(RenderMeshUpdateLink& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    auto& mesh = *reinterpret_cast<RenderMesh*>(
        bytes - offsetof(RenderMesh, update_link));
    render_mesh_process_pending_updates(mesh);
}

}  // namespace rb4
