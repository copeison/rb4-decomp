#pragma once

#include "render/core/textures/render_texture_cube.h"

namespace rb4 {

void render_texture_cube_set_base_dispatch(RenderTextureCube& texture);
void render_delete_texture_cube_storage(RenderTextureCube& texture);

}  // namespace rb4
