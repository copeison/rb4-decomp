// Constructs common 2D texture-array state and clears Orbis backend state.
// Reconstructed from eboot.elf at 0x8E6640.
void orbis_texture_array_2d_construct(
    OrbisTextureArray2D* texture,
    const RenderTextureArray2DDescriptor* descriptor) {
    texture_array_2d_construct(texture, descriptor);
    texture->vtable = &orbis_texture_array_2d_vtable;
    clear_orbis_texture_array_2d_backend_state(texture);
}
