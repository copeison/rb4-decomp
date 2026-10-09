#pragma once

#include "render/meshes/RndMesh.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"

namespace rb4 {

const RenderMeshFormatDescriptor* render_mesh_format_descriptor(
    RndVertexType format);

}  // namespace rb4
