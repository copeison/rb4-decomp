#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

struct OrbisResourceSignal;

volatile std::uint32_t* orbis_render_context_allocate_resource_label(
    PS4Context& context);
bool orbis_render_context_recording_graphics(
    const PS4Context& context);
bool orbis_render_context_recording_compute(
    const PS4Context& context);
void orbis_render_context_emit_graphics_resource_signal(
    PS4Context& context,
    volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_emit_compute_resource_signal(
    PS4Context& context,
    volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_track_resource_signal(
    PS4Context& context,
    const OrbisResourceSignal& signal);
OrbisResourceSignal* orbis_render_context_find_resource_signal(
    PS4Context& context,
    const void* resource);
void orbis_render_context_emit_graphics_resource_wait(
    PS4Context& context,
    const volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_emit_compute_resource_wait(
    PS4Context& context,
    const volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_remove_resource_signal_group(
    PS4Context& context,
    const volatile std::uint32_t* label);

}  // namespace rb4
