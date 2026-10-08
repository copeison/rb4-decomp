#pragma once

#include "render/core/textures/render_texture_array_2d.h"

namespace rb4 {

void render_texture_array_2d_set_base_dispatch(
    RenderTextureArray2D& texture);
void render_delete_texture_array_2d_storage(
    RenderTextureArray2D& texture);

}  // namespace rb4
