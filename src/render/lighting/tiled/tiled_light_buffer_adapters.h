#pragma once

#include "render/lighting/tiled/tiled_light_buffers.h"

namespace rb4 {

struct RenderComputeBuffer;
struct RenderLightingSystem;

RenderComputeBuffer*& render_lighting_tiled_light_buffer(
    RenderLightingSystem& system,
    TiledLightBufferKind kind);
void render_lighting_initialize_remaining(RenderLightingSystem& system);

}  // namespace rb4
