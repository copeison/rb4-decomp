#include "render/platform/orbis/synchronization/orbis_gpu_timing.h"
#include "renderps4/context/PS4Context.h"

#include <cstdint>

#include "render/platform/orbis/synchronization/orbis_gpu_timing_adapters.h"

using namespace rb4;

namespace rb4 {

namespace {

constexpr float kGpuClockSeconds = 1.25e-9F;

OrbisGpuTimestampEvent timestamp_event_for(
    const PS4Context& context) {
    return orbis_render_context_recording_graphics(context)
        ? OrbisGpuTimestampEvent::kGraphicsComplete
        : OrbisGpuTimestampEvent::kComputeComplete;
}

}  // namespace

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8EBA20.
void PS4Context::_BeginGpuStatsImpl(unsigned long key) {
    auto& context = *this;
    auto& timestamp = orbis_render_context_acquire_gpu_timestamp(context);
    timestamp.active = true;
    orbis_render_context_emit_gpu_timestamp(
        context, timestamp.begin, timestamp_event_for(context));
    orbis_render_context_store_gpu_timestamp(context, key, timestamp);
}

// Reconstructed from eboot.elf at 0x8EBBB0.
void PS4Context::_EndGpuStatsImpl(unsigned long key) {
    auto& context = *this;
    auto& timestamp = orbis_render_context_find_gpu_timestamp(context, key);
    orbis_render_context_emit_gpu_timestamp(
        context, timestamp.end, timestamp_event_for(context));
}

// Reconstructed from eboot.elf at 0x8EBC70.
RndGpuStatSample PS4Context::_EvalAndRetireGpuStatsImpl(unsigned long key) {
    auto& context = *this;
    auto& timestamp = orbis_render_context_find_gpu_timestamp(context, key);
    const auto elapsed_ticks = *timestamp.end - *timestamp.begin;

    RndGpuStatSample statistics = {};
    statistics.mSeconds =
        static_cast<float>(elapsed_ticks) * kGpuClockSeconds;

    timestamp.active = false;
    orbis_render_context_remove_gpu_timestamp(context, key);
    return statistics;
}
