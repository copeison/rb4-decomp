#pragma once

#include <cstdint>

namespace rb4 {

struct RenderLightingSystem;

enum class TiledLightBufferKind : std::uint32_t {
    kPointLights,
    kSpotLights,
    kDirectionalLights,
    kLightProbes,
    kSliceZeroLightIds,
};

void render_tiled_light_buffers_initialize(RenderLightingSystem& system);
void render_tiled_light_buffers_release(RenderLightingSystem& system);
void render_lighting_rebuild_spot_shadow_depth_array(
    RenderLightingSystem& system);

}  // namespace rb4
