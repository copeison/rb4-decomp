#include "render/platform/orbis/meshes/orbis_skinned_mesh.h"

#include <cstddef>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"
#include "render/platform/orbis/meshes/orbis_mesh_storage.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using SkinnedMeshLayout = OrbisMeshLayout<SkinnedMeshVertex>;

SkinnedMeshVertex default_skinned_vertex() {
    SkinnedMeshVertex vertex{};
    vertex.color[3] = 1.0F;
    return vertex;
}

static_assert(
    sizeof(SkinnedMeshVertex) == 100,
    "unexpected skinned vertex size");
static_assert(
    offsetof(SkinnedMeshLayout, vertices_begin) == 128,
    "unexpected skinned vertex offset");
static_assert(sizeof(SkinnedMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

}  // namespace

// Reconstructed from eboot.elf at 0x8DDC10.
std::size_t orbis_skinned_mesh_vertex_count(const OrbisMesh& mesh) {
    return mesh_vertex_count<SkinnedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DE0F0.
void orbis_skinned_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count) {
    orbis_mesh_grow_vertex_storage<SkinnedMeshVertex>(
        mesh, additional_count, default_skinned_vertex);
}

// Reconstructed from eboot.elf at 0x8DDC40.
void orbis_skinned_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = mesh_layout<SkinnedMeshVertex>(mesh);
    const auto current_count = orbis_skinned_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_skinned_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = vertex_count == 0
        ? layout.vertices_begin
        : layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8DDC90.
void orbis_skinned_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_skinned_mesh_release_vertices(mesh);
}

void orbis_skinned_mesh_release_vertices(OrbisMesh& mesh) {
    orbis_mesh_release_vertex_storage<SkinnedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DDD20.
SkinnedMeshVertex* orbis_skinned_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return mesh_layout<SkinnedMeshVertex>(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8DDD30.
SkinnedMeshVertex* orbis_skinned_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<SkinnedMeshVertex>(mesh);
    const auto vertex_count = orbis_skinned_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(SkinnedMeshVertex) * vertex_count;
    auto* copy = static_cast<SkinnedMeshVertex*>(
        render_allocate_named(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8DDD90.
void orbis_skinned_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<SkinnedMeshVertex>(mesh).base.vertex_count =
        orbis_skinned_mesh_vertex_count(mesh);
    orbis_skinned_mesh_rebuild_vertex_buffers(mesh);
    orbis_skinned_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8DE6D0.
void orbis_skinned_mesh_rebuild_vertex_buffers(OrbisMesh& mesh) {
    orbis_mesh_rebuild_vertex_buffers<SkinnedMeshVertex>(
        mesh, RenderMeshFormat::kSkinned);
}

// Reconstructed from eboot.elf at 0x8DE8E0.
void orbis_skinned_mesh_rebuild_index_buffer(OrbisMesh& mesh) {
    orbis_mesh_rebuild_index_buffer<SkinnedMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DDDD0.
void orbis_skinned_mesh_update_backend(
    OrbisMesh& mesh,
    void* update_context,
    MeshUpdateFlags flags) {
    (void)update_context;
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<SkinnedMeshVertex>(mesh);
    const auto vertex_count = orbis_skinned_mesh_vertex_count(mesh);
    layout.base.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(SkinnedMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
