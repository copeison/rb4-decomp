// Constructs common cube texture-array state and clears Orbis backend state.
// Reconstructed from eboot.elf at 0x8E6640.
void orbis_texture_array_cube_construct(
    OrbisTextureArrayCube* texture,
    const RenderTextureArrayCubeDescriptor* descriptor) {
    texture_array_cube_construct(texture, descriptor);
    texture->vtable = &orbis_texture_array_cube_vtable;
    clear_orbis_texture_array_cube_backend_state(texture);
}
