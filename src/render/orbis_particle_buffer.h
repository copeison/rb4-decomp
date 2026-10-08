#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisParticleBuffer;

OrbisParticleBuffer* orbis_create_particle_buffer(
    std::uint32_t particle_count,
    void* context);
void orbis_particle_buffer_construct(
    OrbisParticleBuffer& buffer,
    std::uint32_t particle_count,
    void* context);

}  // namespace rb4
