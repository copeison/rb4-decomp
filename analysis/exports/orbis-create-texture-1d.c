// Allocates and constructs a 408-byte Orbis 1D texture.
// Reconstructed from eboot.elf at 0x8D8980.
OrbisTexture1D* orbis_create_texture_1d(
    const RenderTexture1DDescriptor* descriptor) {
    OrbisTexture1D* texture = render_allocate(408);
    orbis_texture_1d_construct(texture, descriptor);
    return texture;
}
