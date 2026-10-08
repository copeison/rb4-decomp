#pragma once

#include <cstddef>

#include "orbis_texture_1d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_1d_construct(
    OrbisTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor);
void orbis_texture_1d_clear_backend_state(OrbisTexture1D& texture);

}  // namespace rb4
