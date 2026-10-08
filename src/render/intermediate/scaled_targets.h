#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class ScaledTargetLevel : std::uint32_t {
    kHalf = 1,
    kQuarter = 2,
    kEighth = 3,
};

enum class ScaledTargetLane : std::uint32_t {
    kFirst,
    kSecond,
};

void render_scaled_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_scaled_targets_release(RenderTargetResources& resources);

}  // namespace rb4
