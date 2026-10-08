#include "render/platform/orbis/synchronization/orbis_gpu_timing.h"

#include <cstdint>

#include "render/platform/orbis/synchronization/orbis_gpu_timing_adapters.h"

namespace rb4 {

namespace {

constexpr float kGpuClockSeconds = 1.25e-9F;

OrbisGpuTimestampEvent timestamp_event_for(
    const OrbisRenderContext& context) {
    return orbis_render_context_recording_graphics(context)
        ? OrbisGpuTimestampEvent::kGraphicsComplete
        : OrbisGpuTimestampEvent::kComputeComplete;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8EBA20.
void orbis_render_context_begin_gpu_stat(
    OrbisRenderContext& context,
    std::uint64_t key) {
    auto& timestamp = orbis_render_context_acquire_gpu_timestamp(context);
    timestamp.active = true;
    orbis_render_context_emit_gpu_timestamp(
        context, timestamp.begin, timestamp_event_for(context));
    orbis_render_context_store_gpu_timestamp(context, key, timestamp);
}

// Reconstructed from eboot.elf at 0x8EBBB0.
void orbis_render_context_end_gpu_stat(
    OrbisRenderContext& context,
    std::uint64_t key) {
    auto& timestamp = orbis_render_context_find_gpu_timestamp(context, key);
    orbis_render_context_emit_gpu_timestamp(
        context, timestamp.end, timestamp_event_for(context));
}

// Reconstructed from eboot.elf at 0x8EBC70.
RenderGpuStatistics orbis_render_context_resolve_gpu_stat(
    OrbisRenderContext& context,
    std::uint64_t key) {
    auto& timestamp = orbis_render_context_find_gpu_timestamp(context, key);
    const auto elapsed_ticks = *timestamp.end - *timestamp.begin;

    RenderGpuStatistics statistics = {};
    statistics.elapsed_seconds =
        static_cast<float>(elapsed_ticks) * kGpuClockSeconds;

    timestamp.active = false;
    orbis_render_context_remove_gpu_timestamp(context, key);
    return statistics;
}

}  // namespace rb4
