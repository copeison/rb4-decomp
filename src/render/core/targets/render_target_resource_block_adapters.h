#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/core/targets/render_target_resources.h"
#include "render/lighting/tiled/tiled_light_target_buffers.h"

namespace rb4 {

struct RenderTarget;

void render_target_resource_block_create_partial_frame_state(
    RenderTargetResourceBlock& block);
void* render_target_resource_block_partial_frame_state(
    RenderTargetResourceBlock& block);
void render_target_resource_block_release_partial_frame_state(
    RenderTargetResourceBlock& block);
void render_target_resource_block_release_unclassified_targets(
    RenderTargetResourceBlock& block);
TiledLightTargetResources& render_target_resource_block_tiled_light_resources(
    RenderTargetResourceBlock& block);
const TiledLightTargetResources&
render_target_resource_block_tiled_light_resources(
    const RenderTargetResourceBlock& block);
RenderTarget* render_target_resources_tiled_light_fallback_target(
    RenderTargetResources& resources,
    RenderExtent interpolation_extent);

}  // namespace rb4
