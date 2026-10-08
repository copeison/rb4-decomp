// Allocates and constructs an 832-byte Orbis cube texture.
// Reconstructed from eboot.elf at 0x8D8A10.
OrbisTextureCube* orbis_create_texture_cube(
    const RenderTextureCubeDescriptor* descriptor) {
    OrbisTextureCube* texture = render_allocate(832);
    orbis_texture_cube_construct(texture, descriptor);
    return texture;
}
