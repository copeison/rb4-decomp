#pragma once

#include <cstdint>

namespace rb4 {

struct RenderTargetResources;

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
