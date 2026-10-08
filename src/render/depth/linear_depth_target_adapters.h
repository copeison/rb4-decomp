#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/depth/linear_depth_targets.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_linear_depth_target(
    RenderTargetResources& resources,
    LinearDepthTargetKind kind,
    RenderExtent extent,
    RenderTexture* reusable_target,
    bool register_with_owner);

}  // namespace rb4
