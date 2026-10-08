#pragma once

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

void render_ambient_occlusion_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block);
void render_ambient_occlusion_target_release(
    RenderTargetResourceBlock& block);

}  // namespace rb4
