#include "render/masking/scene_mask_tiles.h"

#include <cstddef>

#include "render/core/meshes/render_mesh.h"
#include "render/core/meshes/render_mesh_adapters.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture_adapters.h"

namespace rb4 {

namespace {

RenderTexture*& target_slot(
    RenderTargetResources& resources,
    SceneMaskTileTargetKind kind) {
    return resources.tiled_scene_mask[static_cast<std::uint32_t>(kind)];
}

RenderTexture* target_slot(
    const RenderTargetResources& resources,
    SceneMaskTileTargetKind kind) {
    return resources.tiled_scene_mask[static_cast<std::uint32_t>(kind)];
}

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderTexture* reusable_target(
    const RenderTargetResources* resources,
    SceneMaskTileTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : target_slot(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    SceneMaskTileTargetKind kind,
    RenderExtent extent) {
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = 1;
    creation_state.values[9] = 1;
    creation_state.values[10] = 10;
    const RenderDataFormatDescriptor format_descriptor{
        8, 10, 0, 1, -1,
    };
    const auto target_flags =
        kind == SceneMaskTileTargetKind::kPrimary ? 4U : 0U;
    auto* target = render_target_resources_create_texture_2d(
        resources,
        "Scene Mask",
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        -1,
        target_flags,
        reusable_target(reusable_resources, kind));
    target_slot(resources, kind) = target;
    resources.registered_resources_begin[
        resources.registered_resource_count++] = target;
}

void release_target(
    RenderTargetResources& resources,
    SceneMaskTileTargetKind kind) {
    auto*& target = target_slot(resources, kind);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
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
    resources.tiled_scene_mask_mesh = mesh;
}

// Reconstructed from the scene-mask tile portion of eboot.elf at 0x6AFFE0.
void render_scene_mask_tiles_release(RenderTargetResources& resources) {
    release_target(resources, SceneMaskTileTargetKind::kPrimary);
    release_target(resources, SceneMaskTileTargetKind::kSecondary);

    auto*& mesh = resources.tiled_scene_mask_mesh;
    if (mesh != nullptr) {
        render_mesh_release_dynamic(*mesh);
        mesh = nullptr;
    }
}

}  // namespace rb4
