#include "render/core/meshes/render_mesh.h"

#include <algorithm>
#include <cstring>
#include <limits>

#include <pthread.h>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/core/system/render_epoch.h"
#include "render/system/RndFactory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

struct RenderMeshDispatch {
    void (*destruct)(RenderMesh& mesh);
    void (*release_dynamic)(RenderMesh& mesh);
    void* draw;
    RenderMeshFormat (*format)(const RenderMesh& mesh);
    std::size_t (*vertex_count)(const RenderMesh& mesh);
    void (*resize_vertices)(RenderMesh& mesh, std::size_t vertex_count);
    void (*release_vertex_storage)(RenderMesh& mesh);
    void* (*vertex_at)(RenderMesh& mesh, std::size_t index);
    void* (*copy_vertices)(const RenderMesh& mesh);
    void (*finalize)(RenderMesh& mesh);
    void (*update)(
        RenderMesh& mesh,
        void* update_context,
        std::uint32_t flags);
    void (*process_pending_updates)(RenderMesh& mesh);
};

struct RenderMeshUpdateLinkDispatch {
    void (*destruct)(RenderMeshUpdateLink& link);
    void (*delete_link)(RenderMeshUpdateLink& link);
    void (*process_pending_updates)(RenderMeshUpdateLink& link);
};

struct RenderMeshUpdateRegistry {
    std::int32_t mutation_depth;
    std::uint32_t reserved_4;
    ScePthreadMutex mutex;
    RenderMeshUpdateLink** begin;
    RenderMeshUpdateLink** end;
    RenderMeshUpdateLink** capacity;
};

void update_link_destruct(RenderMeshUpdateLink& link);

void update_link_delete(RenderMeshUpdateLink& link) {
    update_link_destruct(link);
    MemFree(&link);
}

RenderMeshUpdateLinkDispatch kBaseUpdateLinkDispatch{
    update_link_destruct,
    update_link_delete,
    nullptr,
};

RenderMesh& mesh_from_update_link(RenderMeshUpdateLink& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    return *reinterpret_cast<RenderMesh*>(
        bytes - offsetof(RenderMesh, update_link));
}

void secondary_destruct(RenderMeshUpdateLink& link) {
    render_mesh_destruct(mesh_from_update_link(link));
}

void secondary_delete(RenderMeshUpdateLink& link) {
    render_mesh_delete(mesh_from_update_link(link));
}

RenderMeshUpdateLinkDispatch kMeshUpdateLinkDispatch{
    secondary_destruct,
    secondary_delete,
    render_mesh_process_pending_updates_secondary,
};

RenderMeshDispatch kBaseMeshDispatch{
    render_mesh_destruct,
    render_mesh_delete,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    render_mesh_process_pending_updates,
};

static_assert(offsetof(RenderMeshDispatch, release_dynamic) == 8);
static_assert(offsetof(RenderMeshDispatch, release_vertex_storage) == 48);
static_assert(offsetof(RenderMeshDispatch, finalize) == 72);
static_assert(offsetof(RenderMeshDispatch, update) == 80);
static_assert(sizeof(RenderMeshDispatch) == 12 * sizeof(void*));

void set_base_dispatch(RenderMesh& mesh) {
    mesh.implementation = &kBaseMeshDispatch;
    mesh.update_link.implementation = &kMeshUpdateLinkDispatch;
}

const RenderMeshDispatch& dispatch(const RenderMesh& mesh) {
    return *static_cast<const RenderMeshDispatch*>(mesh.implementation);
}

void update_link_destruct(RenderMeshUpdateLink& link) {
    link.implementation = &kBaseUpdateLinkDispatch;
    auto* registry = static_cast<RenderMeshUpdateRegistry*>(link.registry);
    if (registry == nullptr) {
        return;
    }

    scePthreadMutexLock(&registry->mutex);
    ++registry->mutation_depth;
    auto** position = std::find(registry->begin, registry->end, &link);
    if (position != registry->end) {
        std::memmove(
            position,
            position + 1,
            static_cast<std::size_t>(registry->end - position - 1) *
                sizeof(*position));
        --registry->end;
    }
    link.registry = nullptr;
    --registry->mutation_depth;
    scePthreadMutexUnlock(&registry->mutex);
}

void destroy_triangles(RenderMeshTriangleArray& triangles) {
    if (triangles.begin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            triangles.begin,
            static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(triangles.capacity) -
                reinterpret_cast<std::uint8_t*>(triangles.begin)));
    }
    triangles = {};
}

}  // namespace

// Reconstructed from eboot.elf at 0x5C26D0.
RenderMesh* render_create_mesh(
    RenderMeshFormat format,
    const char* name) {
    auto& system = *render_system_instance();
    return render_system_factory(system)->CreateMesh(format, name);
}

// Reconstructed from eboot.elf at 0x5C2700.
void render_mesh_construct(RenderMesh& mesh, const char* name) {
    mesh.update_link.implementation = &kBaseUpdateLinkDispatch;
    mesh.update_link.registry = nullptr;
    set_base_dispatch(mesh);

    mesh.triangles = {};
    mesh.vertex_count = 0;
    mesh.triangle_count = 0;
    mesh.geometry_frame = -1;
    mesh.vertices_resident = false;
    mesh.triangles_resident = false;
    mesh.vertex_usage_flags = 0;
    mesh.triangle_usage_flags = 0;
    for (auto& value : mesh.metadata_sentinel) {
        value = std::numeric_limits<std::uint32_t>::max();
    }
    mesh.pending_update_flags.store(0, std::memory_order_relaxed);
    mesh.last_used_frame = std::numeric_limits<std::uint64_t>::max();
    mesh.name = name;
}

// Reconstructed from eboot.elf at 0x5C27B0.
void render_mesh_destruct(RenderMesh& mesh) {
    set_base_dispatch(mesh);
    destroy_triangles(mesh.triangles);
    update_link_destruct(mesh.update_link);
}

// Reconstructed from eboot.elf at 0x5C2870.
void render_mesh_delete(RenderMesh& mesh) {
    render_mesh_destruct(mesh);
    MemFree(&mesh);
}

void render_mesh_release_dynamic(RenderMesh& mesh) {
    dispatch(mesh).release_dynamic(mesh);
}

// Reconstructed from eboot.elf at 0x5C2930.
void render_mesh_set_vertices_resident(RenderMesh& mesh, bool resident) {
    mesh.vertices_resident = resident;
}

// Reconstructed from eboot.elf at 0x5C2940.
void render_mesh_set_vertex_usage_flags(
    RenderMesh& mesh,
    std::uint32_t flags) {
    mesh.vertex_usage_flags = flags;
}

// Reconstructed from eboot.elf at 0x5C2950.
void render_mesh_set_triangle_usage_flags(
    RenderMesh& mesh,
    std::uint32_t flags) {
    mesh.triangle_usage_flags = flags;
}

// Reconstructed from eboot.elf at 0x5C2960.
bool render_mesh_requires_vertex_storage(const RenderMesh& mesh) {
    return mesh.vertices_resident || mesh.triangles_resident ||
           (mesh.vertex_usage_flags & 5U) != 0;
}

// Reconstructed from eboot.elf at 0x5C2980.
bool render_mesh_requires_triangle_storage(const RenderMesh& mesh) {
    return mesh.vertices_resident || mesh.triangles_resident ||
           (mesh.triangle_usage_flags & 5U) != 0;
}

// Reconstructed from eboot.elf at 0x5C29A0.
void render_mesh_finalize(RenderMesh& mesh) {
    mesh.triangle_count = static_cast<std::size_t>(
        mesh.triangles.end - mesh.triangles.begin);
    dispatch(mesh).finalize(mesh);

    if (mesh.vertices_resident || mesh.triangles_resident) {
        return;
    }
    if ((mesh.vertex_usage_flags & 5U) == 0) {
        dispatch(mesh).release_vertex_storage(mesh);
        if (mesh.vertices_resident) {
            return;
        }
    }
    if (!mesh.triangles_resident &&
        (mesh.triangle_usage_flags & 5U) == 0) {
        destroy_triangles(mesh.triangles);
    }
}

// Reconstructed from eboot.elf at 0x5C2DF0.
void render_mesh_apply_updates(
    RenderMesh& mesh,
    void* update_context,
    std::uint32_t flags) {
    mesh.last_used_frame = current_render_epoch();
    if ((flags & 2U) != 0) {
        mesh.triangle_count = static_cast<std::size_t>(
            mesh.triangles.end - mesh.triangles.begin);
    }
    dispatch(mesh).update(mesh, update_context, flags);
}

// Reconstructed from eboot.elf at 0x5C2E40.
void render_mesh_process_pending_updates(RenderMesh& mesh) {
    const auto flags =
        mesh.pending_update_flags.load(std::memory_order_relaxed);
    if (flags == 0) {
        return;
    }

    render_mesh_apply_updates(mesh, nullptr, flags);
    mesh.pending_update_flags.exchange(0);
}

// Reconstructed from eboot.elf at 0x5C2EA0.
void render_mesh_process_pending_updates_secondary(RenderMeshUpdateLink& link) {
    render_mesh_process_pending_updates(mesh_from_update_link(link));
}

void render_mesh_resize_triangles(
    RenderMesh& mesh,
    std::size_t triangle_count) {
    auto& triangles = mesh.triangles;
    const auto capacity = triangles.begin == nullptr
        ? 0
        : static_cast<std::size_t>(triangles.capacity - triangles.begin);
    if (triangle_count <= capacity) {
        triangles.end = triangle_count == 0
            ? triangles.begin
            : triangles.begin + triangle_count;
        return;
    }

    const auto current_count = triangles.begin == nullptr
        ? 0
        : static_cast<std::size_t>(triangles.end - triangles.begin);
    const auto new_capacity = std::max(triangle_count, capacity * 2);
    auto* replacement = static_cast<RenderMeshTriangle*>(
        HmxAllocator::gStlAllocator.allocate(new_capacity * sizeof(RenderMeshTriangle)));
    if (current_count != 0) {
        std::memcpy(
            replacement,
            triangles.begin,
            current_count * sizeof(RenderMeshTriangle));
    }
    destroy_triangles(triangles);
    triangles.begin = replacement;
    triangles.end = replacement + triangle_count;
    triangles.capacity = replacement + new_capacity;
}

void render_mesh_resize_position_vertices(
    RenderMesh& mesh,
    std::size_t vertex_count) {
    dispatch(mesh).resize_vertices(mesh, vertex_count);
}

PositionMeshVertex& render_mesh_position_vertex_at(
    RenderMesh& mesh,
    std::size_t index) {
    return *static_cast<PositionMeshVertex*>(
        dispatch(mesh).vertex_at(mesh, index));
}

}  // namespace rb4
