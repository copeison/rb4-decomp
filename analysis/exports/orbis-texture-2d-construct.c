// Constructs the common 2D texture state and clears the Orbis backend state.
// Reconstructed from eboot.elf at 0x8D62C0.
void orbis_texture_2d_construct(
    OrbisTexture2D* texture,
    const RenderTexture2DDescriptor* descriptor) {
    texture_2d_construct(texture, descriptor);
    texture->vtable = &orbis_texture_2d_vtable;
    clear_orbis_texture_2d_backend_state(texture);
}
