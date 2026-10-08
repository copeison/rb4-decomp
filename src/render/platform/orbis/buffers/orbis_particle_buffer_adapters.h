#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/buffers/orbis_particle_buffer.h"

namespace rb4 {

void orbis_particle_buffer_install_vtable(OrbisParticleBuffer& buffer);
void orbis_defer_particle_buffer_release(void* allocation);
void orbis_particle_buffer_allocate_vertex_stream(
    OrbisParticleBuffer& buffer,
    std::uint32_t stream_index,
    std::size_t size,
    const char* allocation_name);
std::uint16_t* orbis_particle_buffer_allocate_index_stream(
    OrbisParticleBuffer& buffer,
    std::size_t size,
    const char* allocation_name);
void particle_buffer_generate_vertices(
    OrbisParticleBuffer& buffer,
    OrbisRenderContext& context,
    void* destination);
void orbis_particle_buffer_bind_vertex_streams(
    const OrbisParticleBuffer& buffer,
    OrbisRenderContext& context);
void orbis_particle_buffer_bind_instance_streams(
    OrbisRenderContext& context,
    const ParticleDrawState& draw_state);
void orbis_particle_buffer_draw_indices(
    OrbisRenderContext& context,
    std::uint32_t index_count,
    const std::uint16_t* indices);

}  // namespace rb4
