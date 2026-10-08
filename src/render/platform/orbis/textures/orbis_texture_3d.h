#pragma once

namespace rb4 {

struct OrbisTexture3D;
struct RenderTexture3DDescriptor;

OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor& descriptor);
void orbis_texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor);

}  // namespace rb4
