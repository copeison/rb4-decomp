#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

struct OrbisGpuDepthRenderTarget;

struct OrbisDepthClearRange {
    std::uint64_t gpu_address = 0;
    std::uint32_t dword_count = 0;
};

bool orbis_depth_target_has_htile(
    const OrbisGpuDepthRenderTarget& target);
bool orbis_depth_target_stencil_clear_range(
    const OrbisGpuDepthRenderTarget& target,
    OrbisDepthClearRange& range);
OrbisDepthClearRange orbis_depth_target_htile_clear_range(
    const OrbisGpuDepthRenderTarget& target);
void orbis_render_context_flush_depth_metadata(
    PS4Context& context);
void orbis_render_context_dispatch_depth_clear(
    PS4Context& context,
    const OrbisDepthClearRange& range,
    std::uint32_t clear_value);
void orbis_render_context_begin_raster_depth_clear(
    PS4Context& context,
    float depth,
    std::uint8_t stencil);
void orbis_render_context_finish_raster_depth_clear(
    PS4Context& context);
void orbis_render_context_bind_depth_clear_shader(
    PS4Context& context);
void orbis_render_context_set_depth_clear_draw_state(
    PS4Context& context,
    bool enabled);
void orbis_render_context_unbind_pixel_shader(
    PS4Context& context);
void orbis_render_context_submit_depth_clear_draw(
    PS4Context& context);

}  // namespace rb4
