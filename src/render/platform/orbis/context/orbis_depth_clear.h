#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisGpuDepthRenderTarget;
struct OrbisRenderContext;

bool orbis_render_context_clear_depth_stencil_target(
    OrbisRenderContext& context,
    const OrbisGpuDepthRenderTarget& target,
    float depth,
    std::uint8_t stencil);
void orbis_render_context_draw_depth_clear(
    OrbisRenderContext& context);

}  // namespace rb4
