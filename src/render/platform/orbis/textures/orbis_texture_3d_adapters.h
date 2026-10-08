#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_3d.h"

namespace rb4 {

void orbis_texture_3d_install_vtable(OrbisTexture3D& texture);
void orbis_texture_3d_initialize_storage(
    OrbisTexture3D& texture,
    const OrbisTexture3D* storage_source);
void orbis_defer_texture_allocation(void* allocation);

}  // namespace rb4
