#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;

bool orbis_render_context_recording_graphics(
    const OrbisRenderContext& context);
bool orbis_render_context_recording_compute(
    const OrbisRenderContext& context);
std::uint64_t orbis_render_context_active_compute_queue(
    const OrbisRenderContext& context);
void orbis_render_context_select_graphics(OrbisRenderContext& context);
void orbis_render_context_select_compute(
    OrbisRenderContext& context,
    std::uint64_t queue_index);
void orbis_render_context_wait_for_render_target(
    OrbisRenderContext& context,
    const void* resource);
void orbis_render_context_resolve_color_metadata(
    OrbisRenderContext& context,
    const void* resource,
    std::uint64_t subresource);
void orbis_render_context_resolve_depth_metadata(
    OrbisRenderContext& context,
    const void* resource,
    std::uint64_t subresource);
void orbis_render_context_emit_transition_completion_wait(
    OrbisRenderContext& context,
    std::uint32_t cache_actions);
void orbis_render_context_flush_transition_caches(
    OrbisRenderContext& context,
    std::uint32_t cache_actions);

}  // namespace rb4
