#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderMeshUpdateLink {
    void* implementation;
    void* registry;
};

struct RenderMeshTriangle {
    std::uint32_t indices[3];
};

struct RenderMeshTriangleArray {
    RenderMeshTriangle* begin;
    RenderMeshTriangle* end;
    RenderMeshTriangle* capacity;
    void* allocator;
};

struct RenderMesh {
    void* implementation;
    RenderMeshUpdateLink update_link;
    RenderMeshTriangleArray triangles;
    std::size_t vertex_count;
    std::size_t triangle_count;
    std::int64_t geometry_frame;
    bool vertices_resident;
    bool triangles_resident;
    std::uint8_t reserved_82[2];
    std::uint32_t vertex_usage_flags;
    std::uint32_t triangle_usage_flags;
    std::uint32_t metadata_sentinel[4];
    std::atomic<std::uint32_t> pending_update_flags;
    std::uint64_t last_used_frame;
    const char* name;
};

static_assert(sizeof(RenderMeshUpdateLink) == 16);
static_assert(sizeof(RenderMeshTriangle) == 12);
static_assert(sizeof(RenderMeshTriangleArray) == 32);
static_assert(sizeof(RenderMesh) == 128);

void render_mesh_construct(RenderMesh& mesh, const char* name);
void render_mesh_destruct(RenderMesh& mesh);
void render_mesh_delete(RenderMesh& mesh);
void render_mesh_set_vertices_resident(RenderMesh& mesh, bool resident);
void render_mesh_set_vertex_usage_flags(
    RenderMesh& mesh,
    std::uint32_t flags);
void render_mesh_set_triangle_usage_flags(
    RenderMesh& mesh,
    std::uint32_t flags);
bool render_mesh_requires_vertex_storage(const RenderMesh& mesh);
bool render_mesh_requires_triangle_storage(const RenderMesh& mesh);
void render_mesh_finalize(RenderMesh& mesh);
void render_mesh_apply_updates(
    RenderMesh& mesh,
    void* update_context,
    std::uint32_t flags);
void render_mesh_process_pending_updates(RenderMesh& mesh);
void render_mesh_process_pending_updates_secondary(RenderMeshUpdateLink& link);

}  // namespace rb4
