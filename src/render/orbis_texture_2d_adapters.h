#pragma once

#include <cstddef>

#include "orbis_texture_2d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_clear_backend_state(OrbisTexture2D& texture);

}  // namespace rb4
