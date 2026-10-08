#pragma once

#include "render/core/textures/render_texture.h"

namespace rb4 {

void render_texture_set_base_dispatch(RenderTexture& texture);
void render_delete_texture_storage(RenderTexture& texture);

}  // namespace rb4
