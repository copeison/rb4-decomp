#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/intermediate/scaled_targets.h"

namespace rb4 {

struct RenderTarget;

RenderTarget* render_target_resources_create_scaled_target(
    RenderTargetResources& resources,
    ScaledTargetLevel level,
    RenderExtent extent,
    RenderTarget* reusable_target);

}  // namespace rb4
