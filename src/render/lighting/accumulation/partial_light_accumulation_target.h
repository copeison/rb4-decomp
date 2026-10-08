#pragma once

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

void render_partial_light_accumulation_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block);
void render_partial_light_accumulation_target_release(
    RenderTargetResourceBlock& block);

}  // namespace rb4
