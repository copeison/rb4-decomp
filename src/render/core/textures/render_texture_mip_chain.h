#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderFloatPixel {
    float red;
    float green;
    float blue;
    float alpha;
};

struct RenderFloatImageView {
    void* implementation;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::uint32_t reserved_14;
    const RenderFloatPixel* pixels;
    void* reserved_20;
};

struct RenderTextureExtent3D {
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
};

static_assert(sizeof(RenderFloatPixel) == 16);
static_assert(sizeof(RenderFloatImageView) == 40);
static_assert(sizeof(RenderTextureExtent3D) == 12);

void render_texture_mip_chain_descriptor_construct(
    RenderTextureMipChainDescriptor& descriptor);
void render_texture_mip_chain_array_construct(
    RenderTextureMipChainArray& mip_chains);
void render_texture_mip_chain_array_reserve(
    RenderTextureMipChainArray& mip_chains,
    std::size_t capacity);
void render_texture_mip_chain_array_append(
    RenderTextureMipChainArray& mip_chains,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data);
void render_texture_mip_chain_array_validate(
    const RenderTextureMipChainArray& mip_chains);
void render_texture_mip_chain_array_destruct(
    RenderTextureMipChainArray& mip_chains);

}  // namespace rb4
