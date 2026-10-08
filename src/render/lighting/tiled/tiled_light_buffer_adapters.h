#pragma once

namespace rb4 {

struct RenderComputeBuffer;
struct RenderLightingSystem;

void render_lighting_set_point_light_buffer(
    RenderLightingSystem& system,
    RenderComputeBuffer* buffer);
void render_lighting_set_spot_light_buffer(
    RenderLightingSystem& system,
    RenderComputeBuffer* buffer);
void render_lighting_set_directional_light_buffer(
    RenderLightingSystem& system,
    RenderComputeBuffer* buffer);
void render_lighting_set_light_probe_buffer(
    RenderLightingSystem& system,
    RenderComputeBuffer* buffer);
void render_lighting_set_slice_zero_light_ids_buffer(
    RenderLightingSystem& system,
    RenderComputeBuffer* buffer);
void render_lighting_initialize_remaining(RenderLightingSystem& system);

}  // namespace rb4
