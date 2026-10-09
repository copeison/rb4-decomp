#pragma once

#include <cstddef>

#include "render/meshes/RndMesh.h"
#include "render/platform/orbis/meshes/orbis_mesh_draw.h"

namespace rb4 {

struct OrbisRenderContext;

void orbis_render_context_draw_transient(
    OrbisRenderContext& context,
    MeshPrimitiveType primitive_type,
    RndVertexType format,
    const void* vertices,
    std::size_t vertex_count);

}  // namespace rb4
