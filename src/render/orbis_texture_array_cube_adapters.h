#pragma once

#include <cstddef>

#include "orbis_texture_array_cube.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_clear_backend_state(
    OrbisTextureArrayCube& texture);

}  // namespace rb4
