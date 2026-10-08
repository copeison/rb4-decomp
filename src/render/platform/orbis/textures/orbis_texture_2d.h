#pragma once

namespace rb4 {

struct OrbisTexture2D;
struct RenderTexture2DDescriptor;

OrbisTexture2D* orbis_create_texture_2d(
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor);

}  // namespace rb4
