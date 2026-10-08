#include "orbis_particle_buffer.h"

#include <cstddef>
#include <cstdint>

#include "orbis_particle_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisParticleBufferSize = 360;
constexpr std::size_t kVertexBytesPerParticle = 208;
constexpr std::size_t kIndexBytesPerParticle = 12;
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
    particle_buffer_construct(buffer, particle_count, context);
    orbis_particle_buffer_reset_backend(buffer);

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

}  // namespace rb4
