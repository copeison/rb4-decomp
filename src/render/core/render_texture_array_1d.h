#pragma once

#include <cstdint>

#include "render/core/render_texture.h"

namespace rb4 {

struct RenderTextureMipChainDescriptor {
    std::uint8_t storage[80];
};

struct RenderTextureMipChainDescriptorRange {
    const RenderTextureMipChainDescriptor* begin;
    const RenderTextureMipChainDescriptor* end;
    const RenderTextureMipChainDescriptor* capacity;
};

struct RenderTextureArray1DDescriptor {
    std::uint8_t texture_state[144];
    RenderTextureMipChainDescriptorRange mip_chains;
};

struct RenderTextureMipChainArray {
    RenderTextureMipChainState* begin;
    RenderTextureMipChainState* end;
    RenderTextureMipChainState* capacity;
    void* allocator;
};

struct RenderTextureArray1D : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureMipChainArray mip_chains;
};

static_assert(sizeof(RenderTextureMipChainDescriptor) == 80);
static_assert(sizeof(RenderTextureMipChainDescriptorRange) == 24);
static_assert(sizeof(RenderTextureArray1DDescriptor) == 168);
static_assert(sizeof(RenderTextureMipChainArray) == 32);
static_assert(sizeof(RenderTextureArray1D) == 344);

void render_texture_array_1d_construct(
    RenderTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor);
void render_texture_array_1d_destruct(RenderTextureArray1D& texture);
void render_texture_array_1d_delete(RenderTextureArray1D& texture);

}  // namespace rb4
