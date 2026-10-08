#pragma once

#include "render/core/textures/render_texture_1d.h"

namespace rb4 {

void render_texture_1d_set_base_dispatch(RenderTexture1D& texture);
void render_delete_texture_1d_storage(RenderTexture1D& texture);

}  // namespace rb4
