#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_1d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_array_1d_construct(
    OrbisTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor);
void orbis_texture_array_1d_clear_backend_state(
    OrbisTextureArray1D& texture);

}  // namespace rb4
