#pragma once

namespace rb4 {

struct OrbisTextureArrayCube;
struct RenderTextureArrayCubeDescriptor;

OrbisTextureArrayCube* orbis_create_texture_array_cube(
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);

}  // namespace rb4
