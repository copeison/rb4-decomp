#pragma once

namespace rb4 {

struct OrbisTextureCube;
struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;
struct RenderTextureCubeDescriptor;

OrbisTextureCube* orbis_create_texture_cube(
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_destruct(OrbisTextureCube& texture);
void orbis_texture_cube_delete(OrbisTextureCube& texture);
void orbis_texture_cube_initialize_backend(OrbisTextureCube& texture);
const OrbisGpuRenderTarget* orbis_texture_cube_render_target(
    const OrbisTextureCube& texture);
const OrbisGpuDepthRenderTarget* orbis_texture_cube_depth_target(
    const OrbisTextureCube& texture);

}  // namespace rb4
