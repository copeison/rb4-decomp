#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class ShadowContributionTargetKind : std::uint32_t {
    kTextureArray,
    kStencil,
    kScratchPrimary,
    kScratchSecondary,
    kSoftenTilesPrimary,
    kSoftenTilesSecondary,
};

void render_shadow_contribution_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_shadow_contribution_targets_release(
    RenderTargetResources& resources);

}  // namespace rb4
