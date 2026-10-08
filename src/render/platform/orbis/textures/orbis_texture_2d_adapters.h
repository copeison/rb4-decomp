#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_2d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void orbis_texture_2d_install_vtable(OrbisTexture2D& texture);
void render_release(void* allocation);
void orbis_texture_2d_release_auxiliary(void* auxiliary);
void orbis_texture_2d_release_allocation(void* allocation_control);
void orbis_texture_2d_initialize_depth_storage(OrbisTexture2D& texture);
void orbis_texture_2d_initialize_color_storage(
    OrbisTexture2D& texture,
    const OrbisTexture2D* storage_source);
void orbis_texture_2d_flip_active_storage(OrbisTexture2D& texture);
void orbis_texture_2d_upload_active_mips(OrbisTexture2D& texture);
void render_delete_texture_2d_storage(RenderTexture2D& texture);
const void* orbis_texture_2d_binding_view(
    const OrbisTexture2D& texture,
    std::uint32_t flags);

}  // namespace rb4
