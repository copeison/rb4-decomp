#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_1d.h"

namespace rb4 {

void orbis_texture_array_1d_install_vtable(
    OrbisTextureArray1D& texture);
void orbis_texture_array_1d_initialize_storage(
    OrbisTextureArray1D& texture);
void orbis_defer_texture_allocation(void* allocation);

}  // namespace rb4
