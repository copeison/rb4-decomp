#pragma once

#include "render/masking/scene_mask_targets.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_scene_mask_target(
    RenderTargetResources& resources,
    SceneMaskTargetKind kind,
    RenderExtent extent,
    RenderTexture* reusable_target);

}  // namespace rb4
