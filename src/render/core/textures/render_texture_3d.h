#pragma once

#include <cstdint>

#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderTexture3DDescriptor {
    RenderTextureDescriptorState texture_state;
    RenderTextureMipChainDescriptor mip_chain;
};

struct RenderTexture3D : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureMipChainState mip_chain;
};

static_assert(sizeof(RenderTexture3DDescriptor) == 224);
static_assert(sizeof(RenderTexture3D) == 392);

void render_texture_3d_construct(
    RenderTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor);
RenderTexture3D* render_create_texture_3d(
    RenderTexture3DDescriptor& descriptor,
    RenderTexture3D* reusable_texture);
void render_texture_3d_destruct(RenderTexture3D& texture);
void render_texture_3d_delete(RenderTexture3D& texture);

}  // namespace rb4
