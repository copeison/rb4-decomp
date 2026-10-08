#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_cube.h"

namespace rb4 {

void orbis_texture_cube_install_vtable(OrbisTextureCube& texture);
void orbis_texture_cube_initialize_color_storage(
    OrbisTextureCube& texture);
void orbis_texture_cube_initialize_depth_storage(
    OrbisTextureCube& texture);
void* orbis_render_target_metadata_allocation(
    const OrbisGpuRenderTarget& target);
void* orbis_render_target_surface_allocation(
    const OrbisGpuRenderTarget& target);
void orbis_defer_texture_allocation(void* allocation);

}  // namespace rb4
