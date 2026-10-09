#include "render/core/buffers/render_particle_buffer.h"

#include "os/memory/MemMgr.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

struct RenderParticleBufferDispatch {
    void (*destruct)(RenderParticleBuffer& buffer);
    void (*delete_buffer)(RenderParticleBuffer& buffer);
    void (*reserved_initialize)();
};

RenderParticleBufferDispatch kBaseParticleBufferDispatch{
    render_particle_buffer_destruct,
    render_particle_buffer_delete,
    nullptr,
};

void set_base_dispatch(RenderParticleBuffer& buffer) {
    buffer.implementation = &kBaseParticleBufferDispatch;
}

}  // namespace

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
    set_base_dispatch(buffer);
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

void render_delete_particle_buffer_storage(RenderParticleBuffer& buffer) {
    MemFree(&buffer);
}

}  // namespace rb4
