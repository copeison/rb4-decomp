#include "orbis_color_texture_mesh.h"

#include <cstddef>
#include <cstring>

#include "orbis_mesh_adapters.h"
#include "orbis_mesh_layout.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

using ColorTextureMeshLayout = OrbisMeshLayout<ColorTextureMeshVertex>;

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
    layout.vertices_end = layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8DB690.
void orbis_color_texture_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_color_texture_mesh_release_vertices(mesh);
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
        render_allocate_named(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8DB790.
void orbis_color_texture_mesh_finalize_backend(OrbisMesh& mesh) {
    mesh_layout<ColorTextureMeshVertex>(mesh).vertex_count =
        orbis_color_texture_mesh_vertex_count(mesh);
    orbis_color_texture_mesh_rebuild_vertex_buffers(mesh);
    orbis_color_texture_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8DB7D0.
void orbis_color_texture_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags) {
    if (!has_mesh_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = mesh_layout<ColorTextureMeshVertex>(mesh);
    const auto vertex_count = orbis_color_texture_mesh_vertex_count(mesh);
    layout.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(ColorTextureMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
