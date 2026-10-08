#include "render/platform/orbis/buffers/orbis_particle_buffer.h"

#include <cstddef>
#include <cstdint>

#include "render/core/buffers/render_particle_buffer_adapters.h"
#include "render/platform/orbis/buffers/orbis_particle_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisParticleBufferSize = 360;
constexpr std::size_t kVertexBytesPerParticle = 208;
constexpr std::size_t kIndexBytesPerParticle = 12;
constexpr std::uint32_t kIndicesPerParticle = 6;
constexpr const char* kParticleBufferAllocationName = "ParticleBuffer";

}  // namespace

// Reconstructed from eboot.elf at 0x8D8BF0.
OrbisParticleBuffer* orbis_create_particle_buffer(
    std::uint32_t particle_count,
    void* context) {
    auto* storage = render_allocate(kOrbisParticleBufferSize);
    auto* buffer = reinterpret_cast<OrbisParticleBuffer*>(storage);
    orbis_particle_buffer_construct(*buffer, particle_count, context);
    return buffer;
}

// Reconstructed from eboot.elf at 0x8E2AA0.
void orbis_particle_buffer_construct(
    OrbisParticleBuffer& buffer,
    std::uint32_t particle_count,
    void* context) {
    render_particle_buffer_construct(buffer, particle_count, context);
    orbis_particle_buffer_install_vtable(buffer);
    for (auto& bank : buffer.vertex_descriptors) {
        for (auto& descriptor : bank) {
            descriptor = {};
        }
    }
    buffer.vertex_allocations[0] = nullptr;
    buffer.vertex_allocations[1] = nullptr;
    buffer.active_bank = 0;
    buffer.descriptor_mask = 0;
    buffer.backend_reserved = 0;
    buffer.indices = nullptr;

    const auto vertex_bytes = kVertexBytesPerParticle * particle_count;
    orbis_particle_buffer_allocate_vertex_stream(
        buffer, 0, vertex_bytes, kParticleBufferAllocationName);
    orbis_particle_buffer_allocate_vertex_stream(
        buffer, 1, vertex_bytes, kParticleBufferAllocationName);

    const auto index_bytes = kIndexBytesPerParticle * particle_count;
    auto* indices = orbis_particle_buffer_allocate_index_stream(
        buffer, index_bytes, kParticleBufferAllocationName);
    for (std::uint32_t particle = 0; particle < particle_count; ++particle) {
        const auto base = static_cast<std::uint16_t>(particle * 4);
        *indices++ = base;
        *indices++ = static_cast<std::uint16_t>(base + 1);
        *indices++ = static_cast<std::uint16_t>(base + 2);
        *indices++ = base;
        *indices++ = static_cast<std::uint16_t>(base + 2);
        *indices++ = static_cast<std::uint16_t>(base + 3);
    }
}

// Reconstructed from eboot.elf at 0x8E2D30.
void orbis_particle_buffer_destruct(OrbisParticleBuffer& buffer) {
    orbis_defer_particle_buffer_release(buffer.vertex_allocations[0]);
    orbis_defer_particle_buffer_release(buffer.vertex_allocations[1]);
    orbis_defer_particle_buffer_release(buffer.indices);
    render_particle_buffer_destruct(buffer);
}

// Reconstructed from eboot.elf at 0x8E2D80.
void orbis_particle_buffer_delete(OrbisParticleBuffer& buffer) {
    orbis_particle_buffer_destruct(buffer);
    render_delete_particle_buffer_storage(buffer);
}

// Reconstructed from eboot.elf at 0x8E2DE0 and inlined at 0x8E2E43.
void orbis_particle_buffer_upload_vertices(
    OrbisParticleBuffer& buffer,
    OrbisRenderContext& context) {
    buffer.active_bank = (buffer.active_bank & 1U) == 0 ? 1 : 0;
    particle_buffer_generate_vertices(
        buffer,
        context,
        buffer.vertex_allocations[buffer.active_bank]);
}

// Reconstructed from eboot.elf at 0x8E2E10.
void orbis_particle_buffer_draw(
    OrbisParticleBuffer& buffer,
    OrbisRenderContext& context,
    const ParticleDrawState& draw_state) {
    orbis_particle_buffer_upload_vertices(buffer, context);

    const auto particle_count = buffer.active_count;
    if (particle_count == 0) {
        return;
    }

    orbis_particle_buffer_bind_vertex_streams(buffer, context);
    orbis_particle_buffer_bind_instance_streams(context, draw_state);

    const auto index_count = kIndicesPerParticle * particle_count;
    orbis_particle_buffer_draw_indices(
        context,
        index_count,
        buffer.indices);
}

}  // namespace rb4
