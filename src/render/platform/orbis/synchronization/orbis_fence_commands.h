#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

bool orbis_render_context_recording_graphics(
    const PS4Context& context);
bool orbis_render_context_recording_compute(
    const PS4Context& context);
void orbis_render_context_emit_graphics_fence_signal(
    PS4Context& context,
    std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_compute_fence_signal(
    PS4Context& context,
    std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_graphics_fence_wait(
    PS4Context& context,
    const std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_compute_fence_wait(
    PS4Context& context,
    const std::uint32_t* address,
    std::uint32_t value);

}  // namespace rb4
