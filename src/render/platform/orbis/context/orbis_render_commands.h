#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;

void orbis_render_context_dispatch(
    OrbisRenderContext& context,
    std::uint32_t group_count_x,
    std::uint32_t group_count_y,
    std::uint32_t group_count_z);
void orbis_render_context_push_debug_marker(
    OrbisRenderContext& context,
    const char* name);
void orbis_render_context_pop_debug_marker(OrbisRenderContext& context);

}  // namespace rb4
