// Common shader construction plus 32 bytes of geometry backend state.
// Reconstructed from eboot.elf at 0x8E40A0.
void orbis_geometry_shader_construct(OrbisShader* shader) {
    shader_construct(shader);
    shader->vtable = &orbis_geometry_shader_vtable;
    clear_geometry_shader_backend_state(shader);
}
