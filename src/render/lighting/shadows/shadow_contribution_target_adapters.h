#pragma once

#include <cstdint>

#include "render/core/frame/render_frame_owner.h"
#include "render/lighting/shadows/shadow_contribution_targets.h"

namespace rb4 {

struct RenderTarget;

RenderTarget*& render_target_resources_shadow_contribution_target(
    RenderTargetResources& resources,
    ShadowContributionTargetKind kind);
RenderTarget* render_target_resources_shadow_contribution_target(
    const RenderTargetResources& resources,
    ShadowContributionTargetKind kind);
RenderTarget* render_target_resources_create_shadow_contribution_target(
    RenderTargetResources& resources,
    ShadowContributionTargetKind kind,
    RenderExtent extent,
    std::uint32_t texture_array_layers,
    RenderTarget* reusable_target);

}  // namespace rb4
