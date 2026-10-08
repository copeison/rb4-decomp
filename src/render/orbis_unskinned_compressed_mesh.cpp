#include "orbis_unskinned_compressed_mesh.h"

#include <cstddef>
#include <cstring>

#include "orbis_mesh_adapters.h"
#include "orbis_mesh_layout.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using UnskinnedCompressedMeshLayout = OrbisMeshLayout<UnskinnedCompressedMeshVertex>;

static_assert(
    sizeof(UnskinnedCompressedMeshVertex) == 52,
    "unexpected unskinned_compressed vertex size");
static_assert(
    offsetof(UnskinnedCompressedMeshLayout, vertices_begin) == 128,
    "unexpected unskinned_compressed vertex offset");
static_assert(sizeof(UnskinnedCompressedMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

}  // namespace

// Reconstructed from eboot.elf at 0x8DF060.
std::size_t orbis_unskinned_compressed_mesh_vertex_count(const OrbisMesh& mesh) {
    return mesh_vertex_count<UnskinnedCompressedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DF090.
void orbis_unskinned_compressed_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = mesh_layout<UnskinnedCompressedMeshVertex>(mesh);
    const auto current_count = orbis_unskinned_compressed_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_unskinned_compressed_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8DF0E0.
void orbis_unskinned_compressed_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_unskinned_compressed_mesh_release_vertices(mesh);
}

// Reconstructed from eboot.elf at 0x8DF170.
UnskinnedCompressedMeshVertex* orbis_unskinned_compressed_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return mesh_layout<UnskinnedCompressedMeshVertex>(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8DF180.
UnskinnedCompressedMeshVertex* orbis_unskinned_compressed_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<UnskinnedCompressedMeshVertex>(mesh);
    const auto vertex_count = orbis_unskinned_compressed_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(UnskinnedCompressedMeshVertex) * vertex_count;
    auto* copy = static_cast<UnskinnedCompressedMeshVertex*>(
        render_allocate_named(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8DF1E0.
void orbis_unskinned_compressed_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<UnskinnedCompressedMeshVertex>(mesh).vertex_count =
        orbis_unskinned_compressed_mesh_vertex_count(mesh);
    orbis_unskinned_compressed_mesh_rebuild_vertex_buffers(mesh);
    orbis_unskinned_compressed_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8DF220.
void orbis_unskinned_compressed_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags) {
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<UnskinnedCompressedMeshVertex>(mesh);
    const auto vertex_count = orbis_unskinned_compressed_mesh_vertex_count(mesh);
    layout.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(UnskinnedCompressedMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
