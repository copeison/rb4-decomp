#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;
struct OrbisResourceSignal;

volatile std::uint32_t* orbis_render_context_allocate_resource_label(
    OrbisRenderContext& context);
bool orbis_render_context_recording_graphics(
    const OrbisRenderContext& context);
bool orbis_render_context_recording_compute(
    const OrbisRenderContext& context);
void orbis_render_context_emit_graphics_resource_signal(
    OrbisRenderContext& context,
    volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_emit_compute_resource_signal(
    OrbisRenderContext& context,
    volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_track_resource_signal(
    OrbisRenderContext& context,
    const OrbisResourceSignal& signal);
OrbisResourceSignal* orbis_render_context_find_resource_signal(
    OrbisRenderContext& context,
    const void* resource);
void orbis_render_context_emit_graphics_resource_wait(
    OrbisRenderContext& context,
    const volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_emit_compute_resource_wait(
    OrbisRenderContext& context,
    const volatile std::uint32_t* label,
    std::uint32_t value);
void orbis_render_context_remove_resource_signal_group(
    OrbisRenderContext& context,
    const volatile std::uint32_t* label);

}  // namespace rb4
