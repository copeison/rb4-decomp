#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_2d.h"

namespace rb4 {

void orbis_texture_array_2d_install_vtable(
    OrbisTextureArray2D& texture);
void orbis_defer_texture_allocation(void* allocation);
void* orbis_color_target_metadata_allocation(void* color_target);
void orbis_texture_array_2d_initialize_depth_storage(
    OrbisTextureArray2D& texture);
void orbis_texture_array_2d_initialize_color_storage(
    OrbisTextureArray2D& texture);
void render_delete_texture_array_2d_storage(
    OrbisTextureArray2D& texture);

}  // namespace rb4
