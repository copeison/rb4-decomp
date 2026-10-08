// Common shader construction plus 24 bytes of pixel backend state.
// Reconstructed from eboot.elf at 0x8E43D0.
void orbis_pixel_shader_construct(OrbisShader* shader) {
    shader_construct(shader);
    shader->vtable = &orbis_pixel_shader_vtable;
    clear_pixel_shader_backend_state(shader);
}
