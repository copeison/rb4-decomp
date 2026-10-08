#pragma once

#include <cstddef>
#include <cstdint>

#include "orbis_particle_buffer.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void particle_buffer_construct(
    OrbisParticleBuffer& buffer,
    std::uint32_t particle_count,
    void* context);
void orbis_particle_buffer_reset_backend(OrbisParticleBuffer& buffer);
void orbis_particle_buffer_allocate_vertex_stream(
    OrbisParticleBuffer& buffer,
    std::uint32_t stream_index,
    std::size_t size,
    const char* allocation_name);
std::uint16_t* orbis_particle_buffer_allocate_index_stream(
    OrbisParticleBuffer& buffer,
    std::size_t size,
    const char* allocation_name);

}  // namespace rb4
