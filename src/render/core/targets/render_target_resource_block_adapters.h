#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/core/targets/render_target_resources.h"
#include "render/lighting/tiled/tiled_light_target_buffers.h"

namespace rb4 {

struct RenderTexture;

void render_target_resource_block_create_partial_frame_state(
    RenderTargetResourceBlock& block);
void render_target_resource_block_release_partial_frame_state(
    RenderTargetResourceBlock& block);
}  // namespace rb4
