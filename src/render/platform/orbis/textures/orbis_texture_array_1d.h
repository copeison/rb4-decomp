#pragma once

namespace rb4 {

struct OrbisTextureArray1D;
struct RenderTextureArray1DDescriptor;

OrbisTextureArray1D* orbis_create_texture_array_1d(
    const RenderTextureArray1DDescriptor& descriptor);
void orbis_texture_array_1d_construct(
    OrbisTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor);

}  // namespace rb4
