// Allocates and constructs a 408-byte Orbis 3D texture.
// Reconstructed from eboot.elf at 0x8D89E0.
OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor* descriptor) {
    OrbisTexture3D* texture = render_allocate(408);
    orbis_texture_3d_construct(texture, descriptor);
    return texture;
}
