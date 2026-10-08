// Creates two particle streams and a six-index quad for every particle.
// Reconstructed from eboot.elf at 0x8E2AA0.
void orbis_particle_buffer_construct(
    OrbisParticleBuffer* buffer,
    unsigned int particle_count,
    void* context) {
    particle_buffer_construct(buffer, particle_count, context);
    create_particle_vertex_stream(buffer, 0, 208 * particle_count);
    create_particle_vertex_stream(buffer, 1, 208 * particle_count);
    create_particle_quad_indices(buffer, 12 * particle_count);
}
