#pragma once

#include "render/core/textures/render_texture_3d.h"

namespace rb4 {

void render_texture_3d_set_base_dispatch(RenderTexture3D& texture);
void render_delete_texture_3d_storage(RenderTexture3D& texture);

}  // namespace rb4
