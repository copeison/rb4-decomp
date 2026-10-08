#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_3d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor);
void orbis_texture_3d_clear_backend_state(OrbisTexture3D& texture);

}  // namespace rb4
