#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class LightAccumulationTargetKind : std::uint32_t {
    kPrimary0,
    kPrimary1,
    kBlurredHalf,
    kBlurredQuarter,
    kBlurredEighth,
};

void render_light_accumulation_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_light_accumulation_targets_release(
    RenderTargetResources& resources);

}  // namespace rb4
