#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_cube.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void orbis_texture_array_cube_install_vtable(
    OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_initialize_storage(
    OrbisTextureArrayCube& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void render_delete_texture_array_cube_storage(
    OrbisTextureArrayCube& texture);

}  // namespace rb4
