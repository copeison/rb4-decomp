#pragma once

#include <cstdint>

namespace rb4 {

struct RenderTargetResources;

enum class SceneMaskTileTargetKind : std::uint32_t {
    kPrimary,
    kSecondary,
};

void render_scene_mask_tiles_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_scene_mask_tiles_release(RenderTargetResources& resources);

}  // namespace rb4
