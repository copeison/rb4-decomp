#pragma once

#include "render/core/frame/render_frame_owner.h"

namespace rb4 {

struct RenderComputeBuffer;
struct RenderTarget;

RenderTarget* render_create_tiled_light_interpolation_target(
    RenderExtent extent,
    RenderTarget* existing_target);
void render_compute_buffer_release_dynamic(RenderComputeBuffer& buffer);
void render_target_release_dynamic(RenderTarget& target);

}  // namespace rb4
