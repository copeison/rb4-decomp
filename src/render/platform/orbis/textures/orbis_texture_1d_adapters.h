#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_1d.h"

namespace rb4 {

void orbis_texture_1d_install_vtable(OrbisTexture1D& texture);
void orbis_texture_1d_initialize_storage(OrbisTexture1D& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_delete_texture_1d_storage(OrbisTexture1D& texture);

}  // namespace rb4
