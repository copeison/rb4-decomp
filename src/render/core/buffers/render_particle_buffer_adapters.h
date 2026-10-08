#pragma once

#include <cstddef>

#include "render/core/buffers/render_particle_buffer.h"

namespace rb4 {

RenderParticleBuffer* render_system_create_particle_buffer(
    std::size_t particle_count,
    void* context);
void render_particle_buffer_set_base_dispatch(RenderParticleBuffer& buffer);
void render_delete_particle_buffer_storage(RenderParticleBuffer& buffer);

}  // namespace rb4
