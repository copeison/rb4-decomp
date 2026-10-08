#include "render/core/buffers/render_particle_buffer.h"

#include "render/core/buffers/render_particle_buffer_adapters.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x6EAFD0.
RenderParticleBuffer* render_create_particle_buffer(
    std::size_t particle_count,
    void* context) {
    auto& factory = *render_system_factory(*render_system_instance());
    return render_factory_create_particle_buffer(
        factory, static_cast<std::uint32_t>(particle_count), context);
}

// Reconstructed from eboot.elf at 0x6EB000.
void render_particle_buffer_construct(
    RenderParticleBuffer& buffer,
    std::size_t particle_count,
    void* context) {
    render_particle_buffer_set_base_dispatch(buffer);
    buffer.capacity = particle_count;
    buffer.active_count = 0;
    buffer.particle_data = nullptr;
    buffer.reserved_state = 0;
    buffer.orientation_mode = -1;
    buffer.sort_mode = -1;
    buffer.velocity_aligned = false;
    buffer.world_space = true;
    buffer.has_rotation_data = false;
    buffer.context = context;
}

// Reconstructed from eboot.elf at 0x6ECB80.
void render_particle_buffer_destruct(RenderParticleBuffer&) {
}

// Reconstructed from eboot.elf at 0x6ECB90.
void render_particle_buffer_delete(RenderParticleBuffer& buffer) {
    render_delete_particle_buffer_storage(buffer);
}

}  // namespace rb4
