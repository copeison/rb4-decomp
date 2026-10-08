#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/core/targets/render_target_resources.h"

namespace rb4 {

std::uint32_t render_target_resources_flags(
    const RenderTargetResources& resources);
RenderExtent render_target_resources_extent(
    const RenderTargetResources& resources);
bool render_target_resources_use_stereo_targets(
    const RenderTargetResources& resources);

}  // namespace rb4
