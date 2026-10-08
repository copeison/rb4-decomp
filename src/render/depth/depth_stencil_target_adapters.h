#pragma once

#include <cstdint>

#include "render/depth/depth_stencil_target.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_depth_stencil_target(
    RenderTargetResources& resources,
    bool use_40_bit_depth_stencil,
    std::int32_t attachment_index,
    RenderTexture* reusable_target,
    bool register_with_owner);

}  // namespace rb4
