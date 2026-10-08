#pragma once

namespace rb4 {

struct OrbisTexture2D;
struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;
struct RenderTexture2DDescriptor;

OrbisTexture2D* orbis_create_texture_2d(
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_destruct(OrbisTexture2D& texture);
void orbis_texture_2d_delete(OrbisTexture2D& texture);
void orbis_texture_2d_initialize_backend(
    OrbisTexture2D& texture,
    const OrbisTexture2D* storage_source);
void orbis_texture_2d_update_gpu_data(OrbisTexture2D& texture);
const OrbisGpuRenderTarget* orbis_texture_2d_render_target(
    const OrbisTexture2D& texture);
const OrbisGpuDepthRenderTarget* orbis_texture_2d_depth_target(
    const OrbisTexture2D& texture);

}  // namespace rb4
