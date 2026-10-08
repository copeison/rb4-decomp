#pragma once

namespace rb4 {

struct OrbisTextureCube;
struct RenderTextureCubeDescriptor;

OrbisTextureCube* orbis_create_texture_cube(
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);

}  // namespace rb4
