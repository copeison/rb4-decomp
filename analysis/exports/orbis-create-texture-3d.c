// Allocates and constructs a 360-byte Orbis 3D texture.
// Reconstructed from eboot.elf at 0x8D8A40.
OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor* descriptor) {
    OrbisTexture3D* texture = render_allocate(360);
    orbis_texture_3d_construct(texture, descriptor);
    return texture;
}
