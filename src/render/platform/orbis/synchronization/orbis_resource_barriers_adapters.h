#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

bool orbis_render_context_recording_graphics(
    const PS4Context& context);
bool orbis_render_context_recording_compute(
    const PS4Context& context);
std::uint64_t orbis_render_context_active_compute_queue(
    const PS4Context& context);
void orbis_render_context_select_graphics(PS4Context& context);
void orbis_render_context_select_compute(
    PS4Context& context,
    std::uint64_t queue_index);
void orbis_render_context_wait_for_render_target(
    PS4Context& context,
    const void* resource);
void orbis_render_context_resolve_color_metadata(
    PS4Context& context,
    const void* resource,
    std::uint64_t subresource);
void orbis_render_context_resolve_depth_metadata(
    PS4Context& context,
    const void* resource,
    std::uint64_t subresource);
void orbis_render_context_emit_transition_completion_wait(
    PS4Context& context,
    std::uint32_t cache_actions);
void orbis_render_context_flush_transition_caches(
    PS4Context& context,
    std::uint32_t cache_actions);

}  // namespace rb4
