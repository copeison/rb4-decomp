#pragma once

#include <cstddef>

#include "orbis_texture_array_2d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_array_2d_construct(
    OrbisTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor);
void orbis_texture_array_2d_clear_backend_state(
    OrbisTextureArray2D& texture);

}  // namespace rb4
