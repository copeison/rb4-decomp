#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class LinearDepthTargetKind : std::uint32_t {
    kLinearDepth,
    kTiledDepthRange,
};

void render_linear_depth_targets_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block);
void render_linear_depth_targets_release(RenderTargetResourceBlock& block);

}  // namespace rb4
