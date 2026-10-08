// Allocates and constructs a 392-byte Orbis 2D texture array.
// Reconstructed from eboot.elf at 0x8D8A70.
OrbisTextureArray2D* orbis_create_texture_array_2d(
    const RenderTextureArray2DDescriptor* descriptor) {
    OrbisTextureArray2D* texture = render_allocate(392);
    orbis_texture_array_2d_construct(texture, descriptor);
    return texture;
}
