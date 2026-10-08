#pragma once

#include <cstddef>

#include "orbis_mesh.h"

namespace rb4 {

struct OrbisRenderCommandContext;
struct OrbisRenderContext;

std::size_t orbis_transient_vertex_buffer_append(
    OrbisRenderContext& context,
    RenderMeshFormat format,
    const void* vertices,
    std::size_t vertex_count);
void orbis_transient_vertex_buffer_bind(
    OrbisRenderContext& context,
    RenderMeshFormat format);
void orbis_bind_default_instance_vertex_buffers(
    OrbisRenderContext& context);
OrbisRenderCommandContext& orbis_active_render_command_context(
    OrbisRenderContext& context);

}  // namespace rb4
