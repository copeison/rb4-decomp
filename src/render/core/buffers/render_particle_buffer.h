#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderParticleBuffer {
    void* implementation;
    std::size_t capacity;
    std::size_t active_count;
    void* particle_data;
    std::uint64_t reserved_state;
    std::int32_t orientation_mode;
    std::int32_t sort_mode;
    bool velocity_aligned;
    bool world_space;
    bool has_rotation_data;
    std::uint8_t reserved_flags[5];
    void* context;
};

static_assert(sizeof(RenderParticleBuffer) == 64);

RenderParticleBuffer* render_create_particle_buffer(
    std::size_t particle_count,
    void* context);
void render_particle_buffer_construct(
    RenderParticleBuffer& buffer,
    std::size_t particle_count,
    void* context);
void render_particle_buffer_destruct(RenderParticleBuffer& buffer);
void render_particle_buffer_delete(RenderParticleBuffer& buffer);
void render_delete_particle_buffer_storage(RenderParticleBuffer& buffer);

}  // namespace rb4
