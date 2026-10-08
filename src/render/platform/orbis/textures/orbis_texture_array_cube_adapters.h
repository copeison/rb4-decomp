#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_cube.h"

namespace rb4 {

void orbis_texture_array_cube_install_vtable(
    OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_initialize_storage(
    OrbisTextureArrayCube& texture);
void orbis_defer_texture_allocation(void* allocation);

}  // namespace rb4
