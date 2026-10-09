#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

struct OrbisGpuTimestampRecord {
    bool active = false;
    std::uint8_t padding[7] = {};
    volatile std::uint64_t* begin = nullptr;
    volatile std::uint64_t* end = nullptr;
};

static_assert(sizeof(OrbisGpuTimestampRecord) == 24);

enum class OrbisGpuTimestampEvent : std::uint32_t {
    kGraphicsComplete = 0x04,
    kComputeComplete = 0x28,
};

bool orbis_render_context_recording_graphics(
    const PS4Context& context);
OrbisGpuTimestampRecord& orbis_render_context_acquire_gpu_timestamp(
    PS4Context& context);
void orbis_render_context_store_gpu_timestamp(
    PS4Context& context,
    std::uint64_t key,
    OrbisGpuTimestampRecord& timestamp);
OrbisGpuTimestampRecord& orbis_render_context_find_gpu_timestamp(
    PS4Context& context,
    std::uint64_t key);
void orbis_render_context_remove_gpu_timestamp(
    PS4Context& context,
    std::uint64_t key);
void orbis_render_context_emit_gpu_timestamp(
    PS4Context& context,
    volatile std::uint64_t* destination,
    OrbisGpuTimestampEvent event);

}  // namespace rb4
