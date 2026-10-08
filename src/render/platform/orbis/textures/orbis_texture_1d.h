#pragma once

namespace rb4 {

struct OrbisTexture1D;
struct RenderTexture1DDescriptor;

OrbisTexture1D* orbis_create_texture_1d(
    const RenderTexture1DDescriptor& descriptor);
void orbis_texture_1d_construct(
    OrbisTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor);
void orbis_texture_1d_destruct(OrbisTexture1D& texture);
void orbis_texture_1d_delete(OrbisTexture1D& texture);
void orbis_texture_1d_initialize_backend(OrbisTexture1D& texture);

}  // namespace rb4
