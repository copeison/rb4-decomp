#pragma once

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_target_resources_create_light_probe_accumulation_target(
    RenderTargetResources& resources,
    RenderTexture* reusable_target);

}  // namespace rb4
