#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/gbuffer/gbuffer_targets.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_gbuffer_target(
    RenderTargetResources& resources,
    GBufferTargetKind kind,
    RenderExtent extent,
    RenderTexture* reusable_target,
    bool register_with_owner);

}  // namespace rb4
