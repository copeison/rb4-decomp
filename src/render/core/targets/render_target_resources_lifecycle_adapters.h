#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

void render_target_resources_set_base_dispatch(
    RenderTargetResources& resources);
void render_target_resources_set_concrete_dispatch(
    RenderTargetResources& resources);

}  // namespace rb4
