#pragma once

#include <cstddef>

#include "render/core/buffers/render_particle_buffer.h"

namespace rb4 {

void render_particle_buffer_set_base_dispatch(RenderParticleBuffer& buffer);
void render_delete_particle_buffer_storage(RenderParticleBuffer& buffer);

}  // namespace rb4
