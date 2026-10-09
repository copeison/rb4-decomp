#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

struct OrbisGpuDepthRenderTarget;

bool orbis_render_context_clear_depth_stencil_target(
    PS4Context& context,
    const OrbisGpuDepthRenderTarget& target,
    float depth,
    std::uint8_t stencil);
void orbis_render_context_draw_depth_clear(
    PS4Context& context);

}  // namespace rb4
