#include "render/lighting/shadows/shadow_contribution_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format_adapters.h"
#include "render/core/textures/render_texture.h"
#include "render/core/textures/render_texture_adapters.h"

namespace rb4 {

namespace {

RenderTexture*& target_slot(
    RenderTargetResources& resources,
    ShadowContributionTargetKind kind) {
    switch (kind) {
    case ShadowContributionTargetKind::kTextureArray:
        return resources.shadow_contribution_texture_array;
    case ShadowContributionTargetKind::kStencil:
        return resources.shadow_contribution_stencil;
    case ShadowContributionTargetKind::kScratchPrimary:
        return resources.shadow_contribution_scratch[0];
    case ShadowContributionTargetKind::kScratchSecondary:
        return resources.shadow_contribution_scratch[1];
    case ShadowContributionTargetKind::kSoftenTilesPrimary:
        return resources.shadow_soften_tiles[0];
    case ShadowContributionTargetKind::kSoftenTilesSecondary:
        return resources.shadow_soften_tiles[1];
    }
    return resources.shadow_contribution_texture_array;
}

RenderTexture* target_slot(
    const RenderTargetResources& resources,
    ShadowContributionTargetKind kind) {
    switch (kind) {
    case ShadowContributionTargetKind::kTextureArray:
        return resources.shadow_contribution_texture_array;
    case ShadowContributionTargetKind::kStencil:
        return resources.shadow_contribution_stencil;
    case ShadowContributionTargetKind::kScratchPrimary:
        return resources.shadow_contribution_scratch[0];
    case ShadowContributionTargetKind::kScratchSecondary:
        return resources.shadow_contribution_scratch[1];
    case ShadowContributionTargetKind::kSoftenTilesPrimary:
        return resources.shadow_soften_tiles[0];
    case ShadowContributionTargetKind::kSoftenTilesSecondary:
        return resources.shadow_soften_tiles[1];
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
    ShadowContributionTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : target_slot(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    ShadowContributionTargetKind kind,
    RenderExtent extent,
    std::uint32_t texture_array_layers = 1) {
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = static_cast<std::uint32_t>(
        render_texture_default_address_mode(27));
    creation_state.values[9] =
        kind == ShadowContributionTargetKind::kTextureArray
        ? static_cast<std::uint32_t>(render_texture_default_filter_mode(27))
        : 1U;
    creation_state.values[10] = 10;

    RenderDataFormatDescriptor format_descriptor{8, 10, 0, 1, -1};
    const char* name = "Shadow Soften Tiles";
    std::uint32_t target_flags = 0;
    if (kind == ShadowContributionTargetKind::kTextureArray) {
        name = "Shadow Contrib TexArray";
    } else if (kind == ShadowContributionTargetKind::kStencil) {
        creation_state.values[0] = 2;
        format_descriptor = {24, 11, 0, 1, -1};
        name = "Shadow Contrib Stencil";
        target_flags = 16;
    } else if (
        kind == ShadowContributionTargetKind::kScratchPrimary ||
        kind == ShadowContributionTargetKind::kScratchSecondary) {
        format_descriptor = {64, 4, 2, 1, -1};
        name = "Shadow Contrib Scratch";
    }

    const auto data_format = render_data_format_resolve(format_descriptor, 7);
    auto* target = kind == ShadowContributionTargetKind::kTextureArray
        ? render_target_resources_create_texture_array_2d(
              resources,
              name,
              creation_state,
              data_format,
              extent,
              texture_array_layers,
              -1,
              target_flags,
              reusable_target(reusable_resources, kind))
        : render_target_resources_create_texture_2d(
              resources,
              name,
              creation_state,
              data_format,
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
    ShadowContributionTargetKind kind) {
    auto*& target = target_slot(resources, kind);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
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
