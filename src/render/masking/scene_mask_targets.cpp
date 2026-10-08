#include "render/masking/scene_mask_targets.h"

#include <cstddef>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/masking/scene_mask_target_adapters.h"

namespace rb4 {

namespace {

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    SceneMaskTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : render_target_resources_scene_mask_target(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    SceneMaskTargetKind kind,
    RenderExtent extent) {
    render_target_resources_scene_mask_target(resources, kind) =
        render_target_resources_create_scene_mask_target(
            resources,
            kind,
            extent,
            reusable_target(reusable_resources, kind));
}

void release_target(
    RenderTargetResources& resources,
    SceneMaskTargetKind kind) {
    auto*& target = render_target_resources_scene_mask_target(resources, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B1760.
void render_scene_mask_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto extent = render_target_resources_extent(resources);

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
