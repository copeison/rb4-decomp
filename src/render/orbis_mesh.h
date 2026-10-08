#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

enum class RenderMeshFormat : std::uint32_t {
    kColor = 0,
    kColorTexture = 1,
    kUnskinned = 2,
    kSkinned = 3,
    kPositionOnly = 4,
    kParticle = 5,
    kUnskinnedCompressed = 6,
    kSkinnedCompressed = 7,
    kInvalid = 0xFFFFFFFF,
};

struct OrbisMesh;

struct PositionMeshVertex {
    float position[3];
};

enum class MeshUpdateFlags : std::uint32_t {
    kVertices = 1,
};

OrbisMesh* orbis_create_mesh(RenderMeshFormat format, const char* name);
RenderMeshFormat render_mesh_format_from_name(const char* name);

std::size_t orbis_position_mesh_vertex_count(const OrbisMesh& mesh);
void orbis_position_mesh_resize_vertices(
    OrbisMesh& mesh,
    std::size_t vertex_count);
void orbis_position_mesh_clear_vertices(OrbisMesh& mesh);
PositionMeshVertex* orbis_position_mesh_vertex_at(
    OrbisMesh& mesh,
    std::size_t index);
PositionMeshVertex* orbis_position_mesh_copy_vertices(
    const OrbisMesh& mesh);
void orbis_position_mesh_finalize_backend(OrbisMesh& mesh);
void orbis_position_mesh_update_backend(
    OrbisMesh& mesh,
    MeshUpdateFlags flags);

}  // namespace rb4
