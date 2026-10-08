#include "render/platform/orbis/meshes/orbis_skinned_compressed_mesh.h"

#include <cstddef>
#include <cstring>

#include "render/platform/orbis/meshes/orbis_mesh_adapters.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using SkinnedCompressedMeshLayout = OrbisMeshLayout<SkinnedCompressedMeshVertex>;

static_assert(
    sizeof(SkinnedCompressedMeshVertex) == 64,
    "unexpected skinned compressed vertex size");
static_assert(
    offsetof(SkinnedCompressedMeshLayout, vertices_begin) == 128,
    "unexpected skinned compressed vertex offset");
static_assert(sizeof(SkinnedCompressedMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

}  // namespace

// Reconstructed from eboot.elf at 0x8E0500.
std::size_t orbis_skinned_compressed_mesh_vertex_count(const OrbisMesh& mesh) {
    return mesh_vertex_count<SkinnedCompressedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8E0520.
void orbis_skinned_compressed_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = mesh_layout<SkinnedCompressedMeshVertex>(mesh);
    const auto current_count = orbis_skinned_compressed_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_skinned_compressed_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8E0560.
void orbis_skinned_compressed_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_skinned_compressed_mesh_release_vertices(mesh);
}

// Reconstructed from eboot.elf at 0x8E05F0.
SkinnedCompressedMeshVertex* orbis_skinned_compressed_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return mesh_layout<SkinnedCompressedMeshVertex>(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8E0600.
SkinnedCompressedMeshVertex* orbis_skinned_compressed_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<SkinnedCompressedMeshVertex>(mesh);
    const auto vertex_count = orbis_skinned_compressed_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(SkinnedCompressedMeshVertex) * vertex_count;
    auto* copy = static_cast<SkinnedCompressedMeshVertex*>(
        render_allocate_named(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8E0660.
void orbis_skinned_compressed_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<SkinnedCompressedMeshVertex>(mesh).base.vertex_count =
        orbis_skinned_compressed_mesh_vertex_count(mesh);
    orbis_skinned_compressed_mesh_rebuild_vertex_buffers(mesh);
    orbis_skinned_compressed_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8E06A0.
void orbis_skinned_compressed_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags) {
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<SkinnedCompressedMeshVertex>(mesh);
    const auto vertex_count = orbis_skinned_compressed_mesh_vertex_count(mesh);
    layout.base.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(SkinnedCompressedMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
