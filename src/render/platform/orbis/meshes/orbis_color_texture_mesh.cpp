#include "render/platform/orbis/meshes/orbis_color_texture_mesh.h"

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"
#include "render/platform/orbis/meshes/orbis_mesh_storage.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using ColorTextureMeshLayout = OrbisMeshLayout<ColorTextureMeshVertex>;

ColorTextureMeshVertex default_color_texture_vertex() {
    ColorTextureMeshVertex vertex{};
    vertex.color[3] = 1.0F;
    return vertex;
}

static_assert(
    sizeof(ColorTextureMeshVertex) == 36,
    "unexpected color-texture vertex size");
static_assert(
    offsetof(ColorTextureMeshLayout, vertices_begin) == 128,
    "unexpected color-texture vertex offset");
static_assert(sizeof(ColorTextureMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

}  // namespace

// Reconstructed from eboot.elf at 0x8DB610.
std::size_t orbis_color_texture_mesh_vertex_count(const OrbisMesh& mesh) {
    return mesh_vertex_count<ColorTextureMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x44B2F0.
void orbis_color_texture_mesh_grow_vertices(
    OrbisMesh& mesh,
    std::size_t additional_count) {
    orbis_mesh_grow_vertex_storage<ColorTextureMeshVertex>(
        mesh, additional_count, default_color_texture_vertex);
}

// Reconstructed from eboot.elf at 0x8DB640.
void orbis_color_texture_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = mesh_layout<ColorTextureMeshVertex>(mesh);
    const auto current_count = orbis_color_texture_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_color_texture_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = vertex_count == 0
        ? layout.vertices_begin
        : layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8DB690.
void orbis_color_texture_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_color_texture_mesh_release_vertices(mesh);
}

void orbis_color_texture_mesh_release_vertices(OrbisMesh& mesh) {
    orbis_mesh_release_vertex_storage<ColorTextureMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DB720.
ColorTextureMeshVertex* orbis_color_texture_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return mesh_layout<ColorTextureMeshVertex>(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8DB730.
ColorTextureMeshVertex* orbis_color_texture_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<ColorTextureMeshVertex>(mesh);
    const auto vertex_count = orbis_color_texture_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(ColorTextureMeshVertex) * vertex_count;
    auto* copy = static_cast<ColorTextureMeshVertex*>(
        MemAlloc(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8DB790.
void orbis_color_texture_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<ColorTextureMeshVertex>(mesh).base.vertex_count =
        orbis_color_texture_mesh_vertex_count(mesh);
    orbis_color_texture_mesh_rebuild_vertex_buffers(mesh);
    orbis_color_texture_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8DBE60.
void orbis_color_texture_mesh_rebuild_vertex_buffers(OrbisMesh& mesh) {
    orbis_mesh_rebuild_vertex_buffers<ColorTextureMeshVertex>(
        mesh, RenderMeshFormat::kColorTexture);
}

// Reconstructed from eboot.elf at 0x8DC070.
void orbis_color_texture_mesh_rebuild_index_buffer(OrbisMesh& mesh) {
    orbis_mesh_rebuild_index_buffer<ColorTextureMeshVertex>(mesh);
}

// Reconstructed from eboot.elf at 0x8DB7D0.
void orbis_color_texture_mesh_update_backend(
    OrbisMesh& mesh,
    void* update_context,
    MeshUpdateFlags flags) {
    (void)update_context;
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<ColorTextureMeshVertex>(mesh);
    const auto vertex_count = orbis_color_texture_mesh_vertex_count(mesh);
    layout.base.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(ColorTextureMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
