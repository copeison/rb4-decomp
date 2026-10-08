#pragma once

#include <cstdint>

#include "render/core/context/render_context.h"

namespace rb4 {

struct OrbisRenderContext;

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
