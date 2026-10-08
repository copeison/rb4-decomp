// Allocates and constructs a 360-byte Orbis particle buffer.
// Reconstructed from eboot.elf at 0x8D8BF0.
OrbisParticleBuffer* orbis_create_particle_buffer(
    unsigned int particle_count,
    void* context) {
    OrbisParticleBuffer* buffer = render_allocate(360);
    orbis_particle_buffer_construct(buffer, particle_count, context);
    return buffer;
}
