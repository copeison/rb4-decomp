#include "render/masking/scene_mask_targets.h"

#include <cstddef>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture.h"

namespace rb4 {

namespace {

RenderTexture*& target_slot(
    RenderTargetResources& resources,
    SceneMaskTargetKind kind) {
    switch (kind) {
    case SceneMaskTargetKind::kMask:
        return resources.scene_mask;
    case SceneMaskTargetKind::kScratch:
        return resources.scene_mask_scratch;
    case SceneMaskTargetKind::kTile:
        return resources.scene_mask_tiles;
    }
    return resources.scene_mask;
}

RenderTexture* target_slot(
    const RenderTargetResources& resources,
    SceneMaskTargetKind kind) {
    switch (kind) {
    case SceneMaskTargetKind::kMask:
        return resources.scene_mask;
    case SceneMaskTargetKind::kScratch:
        return resources.scene_mask_scratch;
    case SceneMaskTargetKind::kTile:
        return resources.scene_mask_tiles;
    }
    return nullptr;
}

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderTexture* reusable_target(
    const RenderTargetResources* resources,
    SceneMaskTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : target_slot(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    SceneMaskTargetKind kind,
    RenderExtent extent) {
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = static_cast<std::uint32_t>(
        render_texture_default_address_mode(32));
    creation_state.values[9] = static_cast<std::uint32_t>(
        render_texture_default_filter_mode(32));
    creation_state.values[10] = 10;
    const RenderDataFormatDescriptor format_descriptor{
        8, 10, 0, 1, -1,
    };
    const char* name = "Mask Buffer";
    if (kind == SceneMaskTargetKind::kScratch) {
        name = "Mask Scratch Buffer";
    } else if (kind == SceneMaskTargetKind::kTile) {
        name = "Mask Tile Buffer";
        creation_state.values[9] = 1;
    }
    auto* target = render_target_resources_create_texture_2d(
        resources,
        name,
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        -1,
        0,
        reusable_target(reusable_resources, kind));
    target_slot(resources, kind) = target;
    resources.registered_resources_begin[
        resources.registered_resource_count++] = target;
}

void release_target(
    RenderTargetResources& resources,
    SceneMaskTargetKind kind) {
    auto*& target = target_slot(resources, kind);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B1760.
void render_scene_mask_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto extent = resources.extent;

    create_target(
        resources,
        reusable_resources,
        SceneMaskTargetKind::kMask,
        extent);
    create_target(
        resources,
        reusable_resources,
        SceneMaskTargetKind::kScratch,
        extent);

    const auto& settings =
        *render_system_settings(*render_system_instance());
    const auto tile_size =
        static_cast<std::uint32_t>(settings.mask_tile_size);
    const RenderExtent tile_extent{
        divide_round_up(extent.width, tile_size),
        divide_round_up(extent.height, tile_size),
    };
    create_target(
        resources,
        reusable_resources,
        SceneMaskTargetKind::kTile,
        tile_extent);
}

// Reconstructed from the scene-mask portion of eboot.elf at 0x6AFFE0.
void render_scene_mask_targets_release(RenderTargetResources& resources) {
    release_target(resources, SceneMaskTargetKind::kMask);
    release_target(resources, SceneMaskTargetKind::kScratch);
    release_target(resources, SceneMaskTargetKind::kTile);
}

}  // namespace rb4
