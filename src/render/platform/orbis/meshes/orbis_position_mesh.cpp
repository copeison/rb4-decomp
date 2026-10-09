#include "render/platform/orbis/meshes/orbis_position_mesh.h"

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"
#include "render/platform/orbis/meshes/orbis_mesh_storage.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using PositionMeshLayout = OrbisMeshLayout<PositionMeshVertex>;

static_assert(sizeof(PositionMeshVertex) == 12, "unexpected position vertex size");
static_assert(
    offsetof(PositionMeshLayout, vertices_begin) == 128,
    "unexpected position vertex offset");
static_assert(
    offsetof(PositionMeshLayout, vertex_buffers) == 416,
    "unexpected Orbis vertex-buffer offset");
static_assert(sizeof(PositionMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

}  // namespace

// Reconstructed from eboot.elf at 0x8D9100.
std::size_t orbis_position_mesh_vertex_count(const OrbisMesh& mesh) {
    return mesh_vertex_count<PositionMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x6B3ED0.
void orbis_position_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count) {
    orbis_mesh_grow_vertex_storage<PositionMeshVertex>(
        mesh,
        additional_count,
        [] { return PositionMeshVertex{}; });
}

// Reconstructed from eboot.elf at 0x8D9130.
void orbis_position_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = mesh_layout<PositionMeshVertex>(mesh);
    const auto current_count = orbis_position_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_position_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = vertex_count == 0
        ? layout.vertices_begin
        : layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8D9180.
void orbis_position_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_position_mesh_release_vertices(mesh);
}

void orbis_position_mesh_release_vertices(OrbisMesh& mesh) {
    orbis_mesh_release_vertex_storage<PositionMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8D9210.
PositionMeshVertex* orbis_position_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return mesh_layout<PositionMeshVertex>(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8D9220.
PositionMeshVertex* orbis_position_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<PositionMeshVertex>(mesh);
    const auto vertex_count = orbis_position_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(PositionMeshVertex) * vertex_count;
    auto* copy = static_cast<PositionMeshVertex*>(
        MemAlloc(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8D9280.
void orbis_position_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<PositionMeshVertex>(mesh).base.vertex_count =
        orbis_position_mesh_vertex_count(mesh);
    orbis_position_mesh_rebuild_vertex_buffers(mesh);
    orbis_position_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8D9920.
void orbis_position_mesh_rebuild_vertex_buffers(OrbisMesh& mesh) {
    orbis_mesh_rebuild_vertex_buffers<PositionMeshVertex>(
        mesh, RenderMeshFormat::kPositionOnly);
}

// Reconstructed from eboot.elf at 0x8D9B30.
void orbis_position_mesh_rebuild_index_buffer(OrbisMesh& mesh) {
    orbis_mesh_rebuild_index_buffer<PositionMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8D92C0.
void orbis_position_mesh_update_backend(
    OrbisMesh& mesh,
    void* update_context,
    MeshUpdateFlags flags) {
    (void)update_context;
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<PositionMeshVertex>(mesh);
    const auto vertex_count = orbis_position_mesh_vertex_count(mesh);
    layout.base.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(PositionMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
