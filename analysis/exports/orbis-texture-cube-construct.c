// Constructs the common cube texture state and clears the Orbis backend state.
// Reconstructed from eboot.elf at 0x8E6BA0.
void orbis_texture_cube_construct(
    OrbisTextureCube* texture,
    const RenderTextureCubeDescriptor* descriptor) {
    texture_cube_construct(texture, descriptor);
    texture->vtable = &orbis_texture_cube_vtable;
    clear_orbis_texture_cube_backend_state(texture);
}
