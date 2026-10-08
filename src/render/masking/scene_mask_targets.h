#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class SceneMaskTargetKind : std::uint32_t {
    kMask,
    kScratch,
    kTile,
};

void render_scene_mask_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_scene_mask_targets_release(RenderTargetResources& resources);

}  // namespace rb4
