// Common shader construction plus 40 bytes of vertex backend state.
// Reconstructed from eboot.elf at 0x8E46E0.
void orbis_vertex_shader_construct(OrbisShader* shader) {
    shader_construct(shader);
    shader->vtable = &orbis_vertex_shader_vtable;
    clear_vertex_shader_backend_state(shader);
}
