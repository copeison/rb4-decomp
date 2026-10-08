#pragma once

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

void render_light_probe_accumulation_target_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_light_probe_accumulation_target_release(
    RenderTargetResources& resources);

}  // namespace rb4
