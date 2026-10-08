#pragma once

#include "render/core/textures/render_texture_2d.h"

namespace rb4 {

void render_texture_2d_set_base_dispatch(RenderTexture2D& texture);
void render_delete_texture_2d_storage(RenderTexture2D& texture);

}  // namespace rb4
