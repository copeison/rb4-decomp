#pragma once

#include "render/lighting/accumulation/partial_light_accumulation_target.h"

namespace rb4 {

struct RenderTarget;

RenderTarget*& render_target_resource_block_partial_light_accumulation_target(
    RenderTargetResourceBlock& block);
RenderTarget* render_target_resource_block_partial_light_accumulation_target(
    const RenderTargetResourceBlock& block);
}  // namespace rb4
