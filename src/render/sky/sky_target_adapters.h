#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/sky/sky_targets.h"

namespace rb4 {

struct RenderTarget;

RenderTarget* render_target_resources_create_sky_target(
    RenderTargetResources& resources,
    SkyTargetLevel level,
    RenderExtent extent,
    RenderTarget* reusable_target);

}  // namespace rb4
