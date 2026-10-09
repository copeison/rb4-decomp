#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

bool orbis_render_context_recording_graphics(
    const PS4Context& context);
bool orbis_render_context_recording_compute(
    const PS4Context& context);
void orbis_render_context_prepare_graphics_dispatch(
    PS4Context& context);
void orbis_render_context_dispatch_graphics(
    PS4Context& context,
    std::uint32_t group_count_x,
    std::uint32_t group_count_y,
    std::uint32_t group_count_z);
void orbis_render_context_finish_graphics_dispatch(
    PS4Context& context);
void orbis_render_context_prepare_compute_dispatch(
    PS4Context& context);
void orbis_render_context_dispatch_compute(
    PS4Context& context,
    std::uint32_t group_count_x,
    std::uint32_t group_count_y,
    std::uint32_t group_count_z);
void orbis_render_context_push_graphics_debug_marker(
    PS4Context& context,
    const char* name,
    std::uint32_t color);
void orbis_render_context_push_compute_debug_marker(
    PS4Context& context,
    const char* name,
    std::uint32_t color);
void orbis_render_context_pop_graphics_debug_marker(
    PS4Context& context);
void orbis_render_context_pop_compute_debug_marker(
    PS4Context& context);

}  // namespace rb4
