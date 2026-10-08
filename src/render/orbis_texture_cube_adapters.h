#pragma once

#include <cstddef>

#include "orbis_texture_cube.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_clear_backend_state(OrbisTextureCube& texture);

}  // namespace rb4
