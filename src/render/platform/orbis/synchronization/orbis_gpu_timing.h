#pragma once

#include <array>
#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;

struct RenderGpuStatistics {
    float elapsed_seconds = 0.0F;
    std::uint32_t reserved = 0;
    std::array<std::uint64_t, 6> hardware_counters = {};
};

static_assert(sizeof(RenderGpuStatistics) == 56);

void orbis_render_context_begin_gpu_stat(
    OrbisRenderContext& context,
    std::uint64_t key);
void orbis_render_context_end_gpu_stat(
    OrbisRenderContext& context,
    std::uint64_t key);
RenderGpuStatistics orbis_render_context_resolve_gpu_stat(
    OrbisRenderContext& context,
    std::uint64_t key);

}  // namespace rb4
