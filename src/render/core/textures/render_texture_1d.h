#pragma once

#include <cstdint>

#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderTexture1DDescriptor {
    RenderTextureDescriptorState texture_state;
    RenderTextureMipChainDescriptor mip_chain;
};

struct RenderTexture1D : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureMipChainState mip_chain;
};

static_assert(sizeof(RenderTexture1DDescriptor) == 224);
static_assert(sizeof(RenderTexture1D) == 392);

void render_texture_1d_construct(
    RenderTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor);
void render_texture_1d_destruct(RenderTexture1D& texture);
void render_texture_1d_delete(RenderTexture1D& texture);

}  // namespace rb4
