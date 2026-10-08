#pragma once

namespace rb4 {

struct OrbisTextureArray2D;
struct RenderTextureArray2DDescriptor;

OrbisTextureArray2D* orbis_create_texture_array_2d(
    const RenderTextureArray2DDescriptor& descriptor);
void orbis_texture_array_2d_construct(
    OrbisTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor);

}  // namespace rb4
