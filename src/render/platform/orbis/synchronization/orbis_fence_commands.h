#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;
bool orbis_render_context_recording_graphics(
    const OrbisRenderContext& context);
bool orbis_render_context_recording_compute(
    const OrbisRenderContext& context);
void orbis_render_context_emit_graphics_fence_signal(
    OrbisRenderContext& context,
    std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_compute_fence_signal(
    OrbisRenderContext& context,
    std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_graphics_fence_wait(
    OrbisRenderContext& context,
    const std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_compute_fence_wait(
    OrbisRenderContext& context,
    const std::uint32_t* address,
    std::uint32_t value);

}  // namespace rb4
