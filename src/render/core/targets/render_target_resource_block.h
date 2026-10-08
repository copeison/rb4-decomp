#pragma once

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

void render_target_resource_block_initialize(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block);

}  // namespace rb4
