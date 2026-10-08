#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/masking/scene_mask_tiles.h"

namespace rb4 {

struct RenderMesh;
struct RenderTarget;

RenderTarget* render_target_resources_create_scene_mask_tile_target(
    RenderTargetResources& resources,
    SceneMaskTileTargetKind kind,
    RenderExtent extent,
    RenderTarget* reusable_target);

}  // namespace rb4
