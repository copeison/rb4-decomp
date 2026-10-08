#pragma once

namespace rb4 {

struct OrbisTextureArrayCube;
struct RenderTextureArrayCubeDescriptor;

OrbisTextureArrayCube* orbis_create_texture_array_cube(
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_destruct(OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_delete(OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_initialize_backend(
    OrbisTextureArrayCube& texture);

}  // namespace rb4
