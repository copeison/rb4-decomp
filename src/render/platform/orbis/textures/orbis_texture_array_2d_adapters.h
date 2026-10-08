#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_2d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_array_2d_construct(
    OrbisTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor);
void orbis_texture_array_2d_clear_backend_state(
    OrbisTextureArray2D& texture);
void orbis_texture_array_2d_release_backend_state(
    OrbisTextureArray2D& texture);
bool orbis_texture_array_2d_is_depth(
    const OrbisTextureArray2D& texture);
void orbis_texture_array_2d_initialize_depth_storage(
    OrbisTextureArray2D& texture);
void orbis_texture_array_2d_initialize_color_storage(
    OrbisTextureArray2D& texture);
void texture_array_2d_destruct(OrbisTextureArray2D& texture);
void render_delete_texture_array_2d(OrbisTextureArray2D& texture);

}  // namespace rb4
