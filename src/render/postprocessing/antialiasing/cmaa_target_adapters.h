#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/postprocessing/antialiasing/cmaa_targets.h"

namespace rb4 {

struct RenderTexture;

bool render_system_supports_cmaa();
RenderTexture* render_target_resources_create_cmaa_target(
    RenderTargetResources& resources,
    CmaaTargetKind kind,
    RenderExtent extent,
    bool use_64_bit_color,
    RenderTexture* reusable_target);

}  // namespace rb4
