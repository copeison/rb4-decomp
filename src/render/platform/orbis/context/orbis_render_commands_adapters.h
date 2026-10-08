#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;

bool orbis_render_context_recording_graphics(
    const OrbisRenderContext& context);
bool orbis_render_context_recording_compute(
    const OrbisRenderContext& context);
void orbis_render_context_prepare_graphics_dispatch(
    OrbisRenderContext& context);
void orbis_render_context_dispatch_graphics(
    OrbisRenderContext& context,
    std::uint32_t group_count_x,
    std::uint32_t group_count_y,
    std::uint32_t group_count_z);
void orbis_render_context_finish_graphics_dispatch(
    OrbisRenderContext& context);
void orbis_render_context_prepare_compute_dispatch(
    OrbisRenderContext& context);
void orbis_render_context_dispatch_compute(
    OrbisRenderContext& context,
    std::uint32_t group_count_x,
    std::uint32_t group_count_y,
    std::uint32_t group_count_z);
void orbis_render_context_push_graphics_debug_marker(
    OrbisRenderContext& context,
    const char* name,
    std::uint32_t color);
void orbis_render_context_push_compute_debug_marker(
    OrbisRenderContext& context,
    const char* name,
    std::uint32_t color);
void orbis_render_context_pop_graphics_debug_marker(
    OrbisRenderContext& context);
void orbis_render_context_pop_compute_debug_marker(
    OrbisRenderContext& context);

}  // namespace rb4
