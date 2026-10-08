// Common construction and the Orbis compute-shader vtable.
// Reconstructed from eboot.elf at 0x8E3D20.
void orbis_compute_shader_construct(OrbisShader* shader) {
    shader_construct(shader);
    shader->vtable = &orbis_compute_shader_vtable;
}
