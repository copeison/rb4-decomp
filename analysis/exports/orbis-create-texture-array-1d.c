// Allocates and constructs a 360-byte Orbis 1D texture array.
// Reconstructed from eboot.elf at 0x8D8A40.
OrbisTextureArray1D* orbis_create_texture_array_1d(
    const RenderTextureArray1DDescriptor* descriptor) {
    OrbisTextureArray1D* texture = render_allocate(360);
    orbis_texture_array_1d_construct(texture, descriptor);
    return texture;
}
