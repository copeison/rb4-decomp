#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_1d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void orbis_texture_array_1d_install_vtable(
    OrbisTextureArray1D& texture);
void orbis_texture_array_1d_initialize_storage(
    OrbisTextureArray1D& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void render_delete_texture_array_1d_storage(
    OrbisTextureArray1D& texture);

}  // namespace rb4
