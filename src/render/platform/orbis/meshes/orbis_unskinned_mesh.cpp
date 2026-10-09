#include "render/platform/orbis/meshes/orbis_unskinned_mesh.h"

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"
#include "render/platform/orbis/meshes/orbis_mesh_storage.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using UnskinnedMeshLayout = OrbisMeshLayout<UnskinnedMeshVertex>;

UnskinnedMeshVertex default_unskinned_vertex() {
    UnskinnedMeshVertex vertex{};
    vertex.color[3] = 1.0F;
    return vertex;
}

static_assert(
    sizeof(UnskinnedMeshVertex) == 80,
    "unexpected unskinned vertex size");
static_assert(
    offsetof(UnskinnedMeshLayout, vertices_begin) == 128,
    "unexpected unskinned vertex offset");
static_assert(sizeof(UnskinnedMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

}  // namespace

// Reconstructed from eboot.elf at 0x8DC7F0.
std::size_t orbis_unskinned_mesh_vertex_count(const OrbisMesh& mesh) {
    return mesh_vertex_count<UnskinnedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DCCD0.
void orbis_unskinned_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count) {
    orbis_mesh_grow_vertex_storage<UnskinnedMeshVertex>(
        mesh, additional_count, default_unskinned_vertex);
}

// Reconstructed from eboot.elf at 0x8DC820.
void orbis_unskinned_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = mesh_layout<UnskinnedMeshVertex>(mesh);
    const auto current_count = orbis_unskinned_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_unskinned_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = vertex_count == 0
        ? layout.vertices_begin
        : layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8DC870.
void orbis_unskinned_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_unskinned_mesh_release_vertices(mesh);
}

void orbis_unskinned_mesh_release_vertices(OrbisMesh& mesh) {
    orbis_mesh_release_vertex_storage<UnskinnedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DC900.
UnskinnedMeshVertex* orbis_unskinned_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return mesh_layout<UnskinnedMeshVertex>(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8DC910.
UnskinnedMeshVertex* orbis_unskinned_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<UnskinnedMeshVertex>(mesh);
    const auto vertex_count = orbis_unskinned_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(UnskinnedMeshVertex) * vertex_count;
    auto* copy = static_cast<UnskinnedMeshVertex*>(
        MemAlloc(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8DC970.
void orbis_unskinned_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<UnskinnedMeshVertex>(mesh).base.vertex_count =
        orbis_unskinned_mesh_vertex_count(mesh);
    orbis_unskinned_mesh_rebuild_vertex_buffers(mesh);
    orbis_unskinned_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8DD280.
void orbis_unskinned_mesh_rebuild_vertex_buffers(OrbisMesh& mesh) {
    orbis_mesh_rebuild_vertex_buffers<UnskinnedMeshVertex>(
        mesh, RenderMeshFormat::kUnskinned);
}

// Reconstructed from eboot.elf at 0x8DD490.
void orbis_unskinned_mesh_rebuild_index_buffer(OrbisMesh& mesh) {
    orbis_mesh_rebuild_index_buffer<UnskinnedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DC9B0.
void orbis_unskinned_mesh_update_backend(
    OrbisMesh& mesh,
    void* update_context,
    MeshUpdateFlags flags) {
    (void)update_context;
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<UnskinnedMeshVertex>(mesh);
    const auto vertex_count = orbis_unskinned_mesh_vertex_count(mesh);
    layout.base.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(UnskinnedMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
