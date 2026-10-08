#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/lighting/ambient_occlusion/ambient_occlusion_target.h"

namespace rb4 {

struct RenderTarget;

RenderTarget*& render_target_resource_block_ambient_occlusion_target(
    RenderTargetResourceBlock& block);
RenderTarget* render_target_resource_block_ambient_occlusion_target(
    const RenderTargetResourceBlock& block);
RenderTarget* render_target_resources_create_ambient_occlusion_target(
    RenderTargetResources& resources,
    RenderExtent extent,
    RenderTarget* reusable_target,
    bool register_with_owner);

}  // namespace rb4
