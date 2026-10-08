#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisParticleBuffer;
struct OrbisRenderContext;
struct ParticleDrawState;

OrbisParticleBuffer* orbis_create_particle_buffer(
    std::uint32_t particle_count,
    void* context);
void orbis_particle_buffer_construct(
    OrbisParticleBuffer& buffer,
    std::uint32_t particle_count,
    void* context);
void orbis_particle_buffer_destruct(OrbisParticleBuffer& buffer);
void orbis_particle_buffer_delete(OrbisParticleBuffer& buffer);
void orbis_particle_buffer_upload_vertices(
    OrbisParticleBuffer& buffer,
    OrbisRenderContext& context);
void orbis_particle_buffer_draw(
    OrbisParticleBuffer& buffer,
    OrbisRenderContext& context,
    const ParticleDrawState& draw_state);

}  // namespace rb4
