#include "render/lighting/shadows/shadow_contribution_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/lighting/shadows/shadow_contribution_target_adapters.h"

namespace rb4 {

namespace {

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    ShadowContributionTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : render_target_resources_shadow_contribution_target(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    ShadowContributionTargetKind kind,
    RenderExtent extent,
    std::uint32_t texture_array_layers = 1) {
    render_target_resources_shadow_contribution_target(resources, kind) =
        render_target_resources_create_shadow_contribution_target(
            resources,
            kind,
            extent,
            texture_array_layers,
            reusable_target(reusable_resources, kind));
}

void release_target(
    RenderTargetResources& resources,
    ShadowContributionTargetKind kind) {
    auto*& target =
        render_target_resources_shadow_contribution_target(resources, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B1970.
void render_shadow_contribution_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (settings.max_shadow_contrib_buffers == 0) {
        return;
    }

    const auto full_extent = resources.extent;
    const bool uses_reduced_extent =
        full_extent.width > 1920 || full_extent.height > 1080;
    const RenderExtent contribution_extent = uses_reduced_extent
        ? RenderExtent{full_extent.width / 2, full_extent.height / 2}
        : full_extent;

    create_target(
        resources,
        reusable_resources,
        ShadowContributionTargetKind::kTextureArray,
        contribution_extent,
        static_cast<std::uint32_t>(settings.max_shadow_contrib_buffers));

    if (uses_reduced_extent) {
        create_target(
            resources,
            reusable_resources,
            ShadowContributionTargetKind::kStencil,
            contribution_extent);
    }

    create_target(
        resources,
        reusable_resources,
        ShadowContributionTargetKind::kScratchPrimary,
        contribution_extent);
    if (uses_reduced_extent) {
        create_target(
            resources,
            reusable_resources,
            ShadowContributionTargetKind::kScratchSecondary,
            contribution_extent);
    }

    const auto tile_size =
        static_cast<std::uint32_t>(settings.shadow_soften_tile_size);
    const RenderExtent tile_extent{
        divide_round_up(contribution_extent.width, tile_size),
        divide_round_up(contribution_extent.height, tile_size),
    };
    create_target(
        resources,
        reusable_resources,
        ShadowContributionTargetKind::kSoftenTilesPrimary,
        tile_extent);
    create_target(
        resources,
        reusable_resources,
        ShadowContributionTargetKind::kSoftenTilesSecondary,
        tile_extent);
}

// Reconstructed from the shadow-contribution portion of eboot.elf at 0x6AFFE0.
void render_shadow_contribution_targets_release(
    RenderTargetResources& resources) {
    release_target(resources, ShadowContributionTargetKind::kTextureArray);
    release_target(resources, ShadowContributionTargetKind::kStencil);
    release_target(resources, ShadowContributionTargetKind::kScratchPrimary);
    release_target(resources, ShadowContributionTargetKind::kScratchSecondary);
    release_target(resources, ShadowContributionTargetKind::kSoftenTilesPrimary);
    release_target(resources, ShadowContributionTargetKind::kSoftenTilesSecondary);
}

}  // namespace rb4
