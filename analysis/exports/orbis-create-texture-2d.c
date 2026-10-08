// Allocates and constructs a 520-byte Orbis 2D texture.
// Reconstructed from eboot.elf at 0x8D89B0.
OrbisTexture2D* orbis_create_texture_2d(
    const RenderTexture2DDescriptor* descriptor) {
    OrbisTexture2D* texture = render_allocate(520);
    orbis_texture_2d_construct(texture, descriptor);
    return texture;
}
