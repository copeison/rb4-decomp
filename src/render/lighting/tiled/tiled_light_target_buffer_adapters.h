#pragma once

#include "render/core/frame/render_frame_owner.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_create_tiled_light_interpolation_target(
    RenderExtent extent,
    RenderTexture* existing_target);

}  // namespace rb4
