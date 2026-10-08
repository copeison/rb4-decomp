#pragma once

#include <cstddef>

#include "orbis_mesh.h"
#include "orbis_mesh_draw.h"

namespace rb4 {

struct OrbisRenderContext;

void orbis_render_context_draw_transient(
    OrbisRenderContext& context,
    MeshPrimitiveType primitive_type,
    RenderMeshFormat format,
    const void* vertices,
    std::size_t vertex_count);

}  // namespace rb4
