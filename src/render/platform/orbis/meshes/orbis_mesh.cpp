#include "render/platform/orbis/meshes/orbis_mesh.h"

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/platform/orbis/meshes/orbis_color_mesh.h"
#include "render/platform/orbis/meshes/orbis_color_texture_mesh.h"
#include "render/platform/orbis/meshes/orbis_mesh_draw.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"
#include "render/platform/orbis/meshes/orbis_position_mesh.h"
#include "render/platform/orbis/meshes/orbis_skinned_compressed_mesh.h"
#include "render/platform/orbis/meshes/orbis_skinned_mesh.h"
#include "render/platform/orbis/meshes/orbis_unskinned_compressed_mesh.h"
#include "render/platform/orbis/meshes/orbis_unskinned_mesh.h"
#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisMeshSize = 472;

struct OrbisMeshDispatch {
    void (*destruct)(RenderMesh& mesh);
    void (*delete_mesh)(RenderMesh& mesh);
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

struct OrbisMeshUpdateLinkDispatch {
    void (*destruct)(RenderMeshUpdateLink& link);
    void (*delete_link)(RenderMeshUpdateLink& link);
    void (*process_pending_updates)(RenderMeshUpdateLink& link);
};

static_assert(sizeof(OrbisMeshDispatch) == 12 * sizeof(void*));
static_assert(sizeof(OrbisMeshUpdateLinkDispatch) == 3 * sizeof(void*));

OrbisMesh& as_orbis_mesh(RenderMesh& mesh) {
    return reinterpret_cast<OrbisMesh&>(mesh);
}

const OrbisMesh& as_orbis_mesh(const RenderMesh& mesh) {
    return reinterpret_cast<const OrbisMesh&>(mesh);
}

RenderMesh& mesh_from_update_link(RenderMeshUpdateLink& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    return *reinterpret_cast<RenderMesh*>(
        bytes - offsetof(RenderMesh, update_link));
}

template <
    typename Vertex,
    RenderMeshFormat Format,
    auto ResizeVertices,
    auto ReleaseVertices,
    auto VertexAt,
    auto CopyVertices,
    auto Finalize,
    auto Update>
struct MeshOperations {
    static void destruct(RenderMesh& mesh) {
        auto& orbis_mesh = as_orbis_mesh(mesh);
        auto& layout = mesh_layout<Vertex>(orbis_mesh);
        if (g_orbis_render_system != nullptr) {
            orbis_defer_allocation_release(
                *g_orbis_render_system, layout.vertex_buffers[0]);
            orbis_defer_allocation_release(
                *g_orbis_render_system, layout.vertex_buffers[1]);
            orbis_defer_allocation_release(
                *g_orbis_render_system, layout.index_buffer);
        }
        layout.vertex_buffers[0] = nullptr;
        layout.vertex_buffers[1] = nullptr;
        layout.index_buffer = nullptr;

        if (layout.vertices_begin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                layout.vertices_begin,
                static_cast<std::size_t>(
                    reinterpret_cast<std::uint8_t*>(
                        layout.vertices_capacity_end) -
                    reinterpret_cast<std::uint8_t*>(layout.vertices_begin)));
        }
        layout.vertices_begin = nullptr;
        layout.vertices_end = nullptr;
        layout.vertices_capacity_end = nullptr;
        render_mesh_destruct(mesh);
    }

    static void delete_mesh(RenderMesh& mesh) {
        destruct(mesh);
        MemFree(&mesh);
    }

    static void destruct_secondary(RenderMeshUpdateLink& link) {
        destruct(mesh_from_update_link(link));
    }

    static void delete_secondary(RenderMeshUpdateLink& link) {
        delete_mesh(mesh_from_update_link(link));
    }

    static RenderMeshFormat format(const RenderMesh&) {
        return Format;
    }

    static std::size_t vertex_count(const RenderMesh& mesh) {
        return mesh_vertex_count<Vertex>(as_orbis_mesh(mesh));
    }

    static void resize_vertices(
        RenderMesh& mesh,
        std::size_t vertex_count) {
        ResizeVertices(as_orbis_mesh(mesh), vertex_count);
    }

    static void release_vertices(RenderMesh& mesh) {
        ReleaseVertices(as_orbis_mesh(mesh));
    }

    static void* vertex_at(RenderMesh& mesh, std::size_t index) {
        return VertexAt(as_orbis_mesh(mesh), index);
    }

    static void* copy_vertices(const RenderMesh& mesh) {
        return CopyVertices(as_orbis_mesh(mesh));
    }

    static void finalize(RenderMesh& mesh) {
        Finalize(as_orbis_mesh(mesh));
    }

    static void update(
        RenderMesh& mesh,
        void* update_context,
        std::uint32_t flags) {
        Update(
            as_orbis_mesh(mesh),
            update_context,
            static_cast<MeshUpdateFlags>(flags));
    }

    static OrbisMeshDispatch dispatch;
    static OrbisMeshUpdateLinkDispatch update_link_dispatch;
};

template <
    typename Vertex,
    RenderMeshFormat Format,
    auto ResizeVertices,
    auto ReleaseVertices,
    auto VertexAt,
    auto CopyVertices,
    auto Finalize,
    auto Update>
OrbisMeshDispatch MeshOperations<
    Vertex,
    Format,
    ResizeVertices,
    ReleaseVertices,
    VertexAt,
    CopyVertices,
    Finalize,
    Update>::dispatch{
    MeshOperations::destruct,
    MeshOperations::delete_mesh,
    reinterpret_cast<void*>(&orbis_mesh_draw),
    MeshOperations::format,
    MeshOperations::vertex_count,
    MeshOperations::resize_vertices,
    MeshOperations::release_vertices,
    MeshOperations::vertex_at,
    MeshOperations::copy_vertices,
    MeshOperations::finalize,
    MeshOperations::update,
    render_mesh_process_pending_updates,
};

template <
    typename Vertex,
    RenderMeshFormat Format,
    auto ResizeVertices,
    auto ReleaseVertices,
    auto VertexAt,
    auto CopyVertices,
    auto Finalize,
    auto Update>
OrbisMeshUpdateLinkDispatch MeshOperations<
    Vertex,
    Format,
    ResizeVertices,
    ReleaseVertices,
    VertexAt,
    CopyVertices,
    Finalize,
    Update>::update_link_dispatch{
    MeshOperations::destruct_secondary,
    MeshOperations::delete_secondary,
    render_mesh_process_pending_updates_secondary,
};

using PositionMeshOperations = MeshOperations<
    PositionMeshVertex,
    RenderMeshFormat::kPositionOnly,
    orbis_position_mesh_resize_vertices,
    orbis_position_mesh_clear_vertices,
    orbis_position_mesh_vertex_at,
    orbis_position_mesh_copy_vertices,
    orbis_position_mesh_finalize_backend,
    orbis_position_mesh_update_backend>;
using ColorMeshOperations = MeshOperations<
    ColorMeshVertex,
    RenderMeshFormat::kColor,
    orbis_color_mesh_resize_vertices,
    orbis_color_mesh_clear_vertices,
    orbis_color_mesh_vertex_at,
    orbis_color_mesh_copy_vertices,
    orbis_color_mesh_finalize_backend,
    orbis_color_mesh_update_backend>;
using ColorTextureMeshOperations = MeshOperations<
    ColorTextureMeshVertex,
    RenderMeshFormat::kColorTexture,
    orbis_color_texture_mesh_resize_vertices,
    orbis_color_texture_mesh_clear_vertices,
    orbis_color_texture_mesh_vertex_at,
    orbis_color_texture_mesh_copy_vertices,
    orbis_color_texture_mesh_finalize_backend,
    orbis_color_texture_mesh_update_backend>;
using UnskinnedMeshOperations = MeshOperations<
    UnskinnedMeshVertex,
    RenderMeshFormat::kUnskinned,
    orbis_unskinned_mesh_resize_vertices,
    orbis_unskinned_mesh_clear_vertices,
    orbis_unskinned_mesh_vertex_at,
    orbis_unskinned_mesh_copy_vertices,
    orbis_unskinned_mesh_finalize_backend,
    orbis_unskinned_mesh_update_backend>;
using SkinnedMeshOperations = MeshOperations<
    SkinnedMeshVertex,
    RenderMeshFormat::kSkinned,
    orbis_skinned_mesh_resize_vertices,
    orbis_skinned_mesh_clear_vertices,
    orbis_skinned_mesh_vertex_at,
    orbis_skinned_mesh_copy_vertices,
    orbis_skinned_mesh_finalize_backend,
    orbis_skinned_mesh_update_backend>;
using UnskinnedCompressedMeshOperations = MeshOperations<
    UnskinnedCompressedMeshVertex,
    RenderMeshFormat::kUnskinnedCompressed,
    orbis_unskinned_compressed_mesh_resize_vertices,
    orbis_unskinned_compressed_mesh_clear_vertices,
    orbis_unskinned_compressed_mesh_vertex_at,
    orbis_unskinned_compressed_mesh_copy_vertices,
    orbis_unskinned_compressed_mesh_finalize_backend,
    orbis_unskinned_compressed_mesh_update_backend>;
using SkinnedCompressedMeshOperations = MeshOperations<
    SkinnedCompressedMeshVertex,
    RenderMeshFormat::kSkinnedCompressed,
    orbis_skinned_compressed_mesh_resize_vertices,
    orbis_skinned_compressed_mesh_clear_vertices,
    orbis_skinned_compressed_mesh_vertex_at,
    orbis_skinned_compressed_mesh_copy_vertices,
    orbis_skinned_compressed_mesh_finalize_backend,
    orbis_skinned_compressed_mesh_update_backend>;

template <typename Operations>
void initialize_mesh(OrbisMesh& mesh) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&mesh);
    std::memset(
        bytes + sizeof(RenderMesh),
        0,
        kOrbisMeshSize - sizeof(RenderMesh));
    mesh.implementation = &Operations::dispatch;
    mesh.update_link.implementation = &Operations::update_link_dispatch;
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

void orbis_mesh_set_format_backend_defaults(
    OrbisMesh& mesh,
    RenderMeshFormat format) {
    switch (format) {
    case RenderMeshFormat::kColor:
        initialize_mesh<ColorMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kColorTexture:
        initialize_mesh<ColorTextureMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kUnskinned:
        initialize_mesh<UnskinnedMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kSkinned:
        initialize_mesh<SkinnedMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kPositionOnly:
        initialize_mesh<PositionMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kUnskinnedCompressed:
        initialize_mesh<UnskinnedCompressedMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kSkinnedCompressed:
        initialize_mesh<SkinnedCompressedMeshOperations>(mesh);
        break;
    case RenderMeshFormat::kParticle:
    case RenderMeshFormat::kInvalid:
        break;
    }
}

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

    auto* storage = operator new(kOrbisMeshSize);
    auto* mesh = reinterpret_cast<OrbisMesh*>(storage);
    render_mesh_construct(*mesh, name);
    orbis_mesh_set_format_backend_defaults(*mesh, format);
    return mesh;
}

}  // namespace rb4
