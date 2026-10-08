#pragma once

#include "render/core/textures/render_texture_array_1d.h"

namespace rb4 {

void render_texture_array_1d_set_base_dispatch(
    RenderTextureArray1D& texture);
void render_delete_texture_array_1d_storage(
    RenderTextureArray1D& texture);

}  // namespace rb4
