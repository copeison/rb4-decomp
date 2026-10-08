#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisGpuDepthRenderTarget;
struct OrbisRenderContext;

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
    OrbisRenderContext& context);
void orbis_render_context_dispatch_depth_clear(
    OrbisRenderContext& context,
    const OrbisDepthClearRange& range,
    std::uint32_t clear_value);
void orbis_render_context_begin_raster_depth_clear(
    OrbisRenderContext& context,
    float depth,
    std::uint8_t stencil);
void orbis_render_context_finish_raster_depth_clear(
    OrbisRenderContext& context);
void orbis_render_context_bind_depth_clear_shader(
    OrbisRenderContext& context);
void orbis_render_context_set_depth_clear_draw_state(
    OrbisRenderContext& context,
    bool enabled);
void orbis_render_context_unbind_pixel_shader(
    OrbisRenderContext& context);
void orbis_render_context_submit_depth_clear_draw(
    OrbisRenderContext& context);

}  // namespace rb4
