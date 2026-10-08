// Constructs common 3D texture state and clears Orbis backend state.
// Reconstructed from eboot.elf at 0x8E5870.
void orbis_texture_3d_construct(
    OrbisTexture3D* texture,
    const RenderTexture3DDescriptor* descriptor) {
    texture_3d_construct(texture, descriptor);
    texture->vtable = &orbis_texture_3d_vtable;
    clear_orbis_texture_3d_backend_state(texture);
}
