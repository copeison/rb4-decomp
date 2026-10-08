#include "render/masking/scene_mask_tiles.h"

#include <cstddef>

#include "render/core/meshes/render_mesh.h"
#include "render/core/meshes/render_mesh_adapters.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/masking/scene_mask_tile_adapters.h"

namespace rb4 {

namespace {

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    SceneMaskTileTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : render_target_resources_scene_mask_tile_target(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    SceneMaskTileTargetKind kind,
    RenderExtent extent) {
    render_target_resources_scene_mask_tile_target(resources, kind) =
        render_target_resources_create_scene_mask_tile_target(
            resources,
            kind,
            extent,
            reusable_target(reusable_resources, kind));
}

void release_target(
    RenderTargetResources& resources,
    SceneMaskTileTargetKind kind) {
    auto*& target =
        render_target_resources_scene_mask_tile_target(resources, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

void build_tile_mesh(
    RenderMesh& mesh,
    RenderExtent extent,
    std::uint32_t tile_size,
    RenderExtent tile_extent) {
    const auto tile_count =
        static_cast<std::size_t>(tile_extent.width) * tile_extent.height;
    render_mesh_resize_position_vertices(mesh, tile_count * 4);
    render_mesh_resize_triangles(mesh, tile_count * 2);

    std::size_t tile_index = 0;
    for (std::uint32_t row = 0; row < tile_extent.height; ++row) {
        const auto top_pixel = row * tile_size;
        const auto bottom_pixel =
            (row + 1) * tile_size < extent.height
            ? (row + 1) * tile_size
            : extent.height;
        const auto top = 1.0F -
            2.0F * static_cast<float>(top_pixel) / extent.height;
        const auto bottom = 1.0F -
            2.0F * static_cast<float>(bottom_pixel) / extent.height;

        for (std::uint32_t column = 0;
             column < tile_extent.width;
             ++column, ++tile_index) {
            const auto left_pixel = column * tile_size;
            const auto right_pixel =
                (column + 1) * tile_size < extent.width
                ? (column + 1) * tile_size
                : extent.width;
            const auto left = 2.0F * static_cast<float>(left_pixel) /
                                  extent.width -
                              1.0F;
            const auto right = 2.0F * static_cast<float>(right_pixel) /
                                   extent.width -
                               1.0F;
            const auto vertex_base = tile_index * 4;
            render_mesh_position_vertex_at(mesh, vertex_base + 0) =
                {{left, top, 0.0F}};
            render_mesh_position_vertex_at(mesh, vertex_base + 1) =
                {{right, top, 0.0F}};
            render_mesh_position_vertex_at(mesh, vertex_base + 2) =
                {{left, bottom, 0.0F}};
            render_mesh_position_vertex_at(mesh, vertex_base + 3) =
                {{right, bottom, 0.0F}};

            const auto triangle_base = tile_index * 2;
            mesh.triangles.begin[triangle_base + 0] = {{
                static_cast<std::uint32_t>(vertex_base + 0),
                static_cast<std::uint32_t>(vertex_base + 1),
                static_cast<std::uint32_t>(vertex_base + 2),
            }};
            mesh.triangles.begin[triangle_base + 1] = {{
                static_cast<std::uint32_t>(vertex_base + 1),
                static_cast<std::uint32_t>(vertex_base + 3),
                static_cast<std::uint32_t>(vertex_base + 2),
            }};
        }
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B2140.
void render_scene_mask_tiles_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto extent = resources.extent;
    const auto& settings =
        *render_system_settings(*render_system_instance());
    const auto tile_size =
        static_cast<std::uint32_t>(settings.light_tile_size);
    const RenderExtent tile_extent{
        divide_round_up(extent.width, tile_size),
        divide_round_up(extent.height, tile_size),
    };

    create_target(
        resources,
        reusable_resources,
        SceneMaskTileTargetKind::kPrimary,
        tile_extent);
    create_target(
        resources,
        reusable_resources,
        SceneMaskTileTargetKind::kSecondary,
        tile_extent);

    auto* mesh =
        render_create_mesh(RenderMeshFormat::kPositionOnly, "Scene Mask Mesh");
    build_tile_mesh(*mesh, extent, tile_size, tile_extent);
    render_mesh_finalize(*mesh);
    render_target_resources_scene_mask_mesh(resources) = mesh;
}

// Reconstructed from the scene-mask tile portion of eboot.elf at 0x6AFFE0.
void render_scene_mask_tiles_release(RenderTargetResources& resources) {
    release_target(resources, SceneMaskTileTargetKind::kPrimary);
    release_target(resources, SceneMaskTileTargetKind::kSecondary);

    auto*& mesh = render_target_resources_scene_mask_mesh(resources);
    if (mesh != nullptr) {
        render_mesh_release_dynamic(*mesh);
        mesh = nullptr;
    }
}

}  // namespace rb4
