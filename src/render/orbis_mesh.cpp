#include "orbis_mesh.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "orbis_mesh_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;
constexpr const char* kVertexCopyAllocationName = "VerticesCopy";

struct OrbisPositionMeshLayout {
    std::uint8_t base_to_vertex_count[56];
    std::size_t vertex_count;
    std::uint8_t base_tail[64];
    PositionMeshVertex* vertices_begin;
    PositionMeshVertex* vertices_end;
    PositionMeshVertex* vertices_capacity_end;
    std::uint8_t vertex_descriptors[264];
    PositionMeshVertex* vertex_buffers[2];
    std::size_t active_vertex_buffer;
    std::size_t vertex_buffer_capacity;
    std::uint32_t descriptor_mask;
    std::uint32_t index_format;
    void* index_buffer;
    std::size_t index_buffer_capacity;
};

static_assert(sizeof(PositionMeshVertex) == 12, "unexpected position vertex size");
static_assert(
    offsetof(OrbisPositionMeshLayout, vertices_begin) == 128,
    "unexpected position vertex offset");
static_assert(
    offsetof(OrbisPositionMeshLayout, vertex_buffers) == 416,
    "unexpected Orbis vertex-buffer offset");
static_assert(sizeof(OrbisPositionMeshLayout) == kOrbisMeshSize,
              "unexpected Orbis mesh size");

OrbisPositionMeshLayout& position_layout(OrbisMesh& mesh) {
    return reinterpret_cast<OrbisPositionMeshLayout&>(mesh);
}

const OrbisPositionMeshLayout& position_layout(const OrbisMesh& mesh) {
    return reinterpret_cast<const OrbisPositionMeshLayout&>(mesh);
}

bool has_update_flag(MeshUpdateFlags flags, MeshUpdateFlags flag) {
    return (static_cast<std::uint32_t>(flags) &
            static_cast<std::uint32_t>(flag)) != 0;
}

bool is_supported_mesh_format(RenderMeshFormat format) {
    switch (format) {
    case RenderMeshFormat::kColor:
    case RenderMeshFormat::kColorTexture:
    case RenderMeshFormat::kUnskinned:
    case RenderMeshFormat::kSkinned:
    case RenderMeshFormat::kPositionOnly:
    case RenderMeshFormat::kUnskinnedCompressed:
    case RenderMeshFormat::kSkinnedCompressed:
        return true;
    case RenderMeshFormat::kParticle:
    case RenderMeshFormat::kInvalid:
        return false;
    }
    return false;
}

}  // namespace

// Reconstructed from eboot.elf at 0x442930.
RenderMeshFormat render_mesh_format_from_name(const char* name) {
    if (std::strcmp(name, "Color") == 0) {
        return RenderMeshFormat::kColor;
    }
    if (std::strcmp(name, "ColorTex") == 0) {
        return RenderMeshFormat::kColorTexture;
    }
    if (std::strcmp(name, "Unskinned") == 0) {
        return RenderMeshFormat::kUnskinned;
    }
    if (std::strcmp(name, "Skinned") == 0) {
        return RenderMeshFormat::kSkinned;
    }
    if (std::strcmp(name, "PosOnly") == 0) {
        return RenderMeshFormat::kPositionOnly;
    }
    if (std::strcmp(name, "Particle") == 0) {
        return RenderMeshFormat::kParticle;
    }
    if (std::strcmp(name, "UnskinnedCompressed") == 0) {
        return RenderMeshFormat::kUnskinnedCompressed;
    }
    if (std::strcmp(name, "SkinnedCompressed") == 0) {
        return RenderMeshFormat::kSkinnedCompressed;
    }
    return RenderMeshFormat::kInvalid;
}

// Reconstructed from eboot.elf at 0x8D85F0.
OrbisMesh* orbis_create_mesh(RenderMeshFormat format, const char* name) {
    if (!is_supported_mesh_format(format)) {
        return nullptr;
    }

    auto* storage = render_allocate(kOrbisMeshSize);
    auto* mesh = reinterpret_cast<OrbisMesh*>(storage);
    mesh_construct(*mesh, name);
    orbis_mesh_set_format_backend_defaults(*mesh, format);
    return mesh;
}

// Reconstructed from eboot.elf at 0x8D9100.
std::size_t orbis_position_mesh_vertex_count(const OrbisMesh& mesh) {
    const auto& layout = position_layout(mesh);
    return static_cast<std::size_t>(
        layout.vertices_end - layout.vertices_begin);
}

// Reconstructed from eboot.elf at 0x8D9130.
void orbis_position_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count) {
    auto& layout = position_layout(mesh);
    const auto current_count = orbis_position_mesh_vertex_count(mesh);
    if (current_count < vertex_count) {
        orbis_position_mesh_grow_vertices(
            mesh, vertex_count - current_count);
        return;
    }
    layout.vertices_end = layout.vertices_begin + vertex_count;
}

// Reconstructed from eboot.elf at 0x8D9180.
void orbis_position_mesh_clear_vertices(OrbisMesh& mesh) {
    orbis_position_mesh_release_vertices(mesh);
}

// Reconstructed from eboot.elf at 0x8D9210.
PositionMeshVertex* orbis_position_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index) {
    return position_layout(mesh).vertices_begin + index;
}

// Reconstructed from eboot.elf at 0x8D9220.
PositionMeshVertex* orbis_position_mesh_copy_vertices(
    const OrbisMesh& mesh) {
    const auto& layout = position_layout(mesh);
    const auto vertex_count = orbis_position_mesh_vertex_count(mesh);
    if (vertex_count == 0) {
        return nullptr;
    }

    const auto byte_count = sizeof(PositionMeshVertex) * vertex_count;
    auto* copy = static_cast<PositionMeshVertex*>(
        render_allocate_named(byte_count, kVertexCopyAllocationName, 4));
    std::memcpy(copy, layout.vertices_begin, byte_count);
    return copy;
}

// Reconstructed from eboot.elf at 0x8D9280.
void orbis_position_mesh_finalize_backend(OrbisMesh& mesh) {
    position_layout(mesh).vertex_count =
        orbis_position_mesh_vertex_count(mesh);
    orbis_position_mesh_rebuild_vertex_buffers(mesh);
    orbis_position_mesh_rebuild_index_buffer(mesh);
}

// Reconstructed from eboot.elf at 0x8D92C0.
void orbis_position_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags) {
    if (!has_update_flag(flags, MeshUpdateFlags::kVertices)) {
        return;
    }

    auto& layout = position_layout(mesh);
    const auto vertex_count = orbis_position_mesh_vertex_count(mesh);
    layout.vertex_count = vertex_count;
    layout.active_vertex_buffer ^= 1;

    const auto byte_count = sizeof(PositionMeshVertex) * vertex_count;
    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);
}

}  // namespace rb4
