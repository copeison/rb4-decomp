// Allocates and constructs a 360-byte Orbis cube texture array.
// Reconstructed from eboot.elf at 0x8D8AA0.
OrbisTextureArrayCube* orbis_create_texture_array_cube(
    const RenderTextureArrayCubeDescriptor* descriptor) {
    OrbisTextureArrayCube* texture = render_allocate(360);
    orbis_texture_array_cube_construct(texture, descriptor);
    return texture;
}
