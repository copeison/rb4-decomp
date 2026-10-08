#pragma once

#include <cstddef>

#include "orbis_mesh.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void mesh_construct(OrbisMesh& mesh, const char* name);
void orbis_mesh_set_format_backend_defaults(
    OrbisMesh& mesh,
    RenderMeshFormat format);

}  // namespace rb4
