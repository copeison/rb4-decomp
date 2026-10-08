#pragma once

#include "render/core/textures/render_texture_array_cube.h"

namespace rb4 {

void render_texture_array_cube_set_base_dispatch(
    RenderTextureArrayCube& texture);
void render_delete_texture_array_cube_storage(
    RenderTextureArrayCube& texture);

}  // namespace rb4
