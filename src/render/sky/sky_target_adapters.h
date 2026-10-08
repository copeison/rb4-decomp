#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/sky/sky_targets.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_sky_target(
    RenderTargetResources& resources,
    SkyTargetLevel level,
    RenderExtent extent,
    RenderTexture* reusable_target);

}  // namespace rb4
