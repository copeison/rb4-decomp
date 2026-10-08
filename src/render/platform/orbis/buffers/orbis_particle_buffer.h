#pragma once

#include <cstdint>

#include "render/core/render_particle_buffer.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"

namespace rb4 {

struct OrbisRenderContext;
struct ParticleDrawState;

struct OrbisParticleBuffer : RenderParticleBuffer {
    OrbisBufferDescriptor vertex_descriptors[2][kMeshVertexStreamCount];
    void* vertex_allocations[2];
    std::size_t active_bank;
    std::uint32_t descriptor_mask;
    std::uint32_t backend_reserved;
    std::uint16_t* indices;
};

static_assert(sizeof(OrbisParticleBuffer) == 360);

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
