#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class SkyTargetLevel : std::uint32_t {
    kFull,
    kHalf,
    kQuarter,
    kEighth,
};

void render_sky_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_sky_targets_release(RenderTargetResources& resources);

}  // namespace rb4
