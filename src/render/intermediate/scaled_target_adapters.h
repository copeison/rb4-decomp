#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/intermediate/scaled_targets.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_scaled_target(
    RenderTargetResources& resources,
    ScaledTargetLevel level,
    RenderExtent extent,
    RenderTexture* reusable_target);

}  // namespace rb4
