#pragma once

#include <cstdint>

#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderTextureArray2DDescriptor {
    RenderTextureDescriptorState texture_state;
    RenderTextureMipChainDescriptorRange mip_chains;
};

struct RenderTextureArray2D : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureMipChainArray mip_chains;
};

static_assert(sizeof(RenderTextureArray2DDescriptor) == 168);
static_assert(sizeof(RenderTextureArray2D) == 344);

void render_texture_array_2d_construct(
    RenderTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor);
RenderTextureArray2D* render_create_texture_array_2d(
    RenderTextureArray2DDescriptor& descriptor);
void render_texture_array_2d_destruct(RenderTextureArray2D& texture);
void render_texture_array_2d_delete(RenderTextureArray2D& texture);

}  // namespace rb4
