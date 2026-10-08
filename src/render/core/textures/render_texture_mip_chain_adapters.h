#pragma once

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
void render_texture_mip_chain_descriptor_destruct(
    RenderTextureMipChainDescriptor& descriptor);
void render_texture_mip_chain_descriptor_allocate_source(
    RenderTextureMipChainDescriptor& descriptor,
    const RenderTextureExtent3D& extent,
    std::int32_t data_format,
    const void* source_data);
bool render_texture_mip_chain_descriptor_copy_float_image(
    RenderTextureMipChainDescriptor& descriptor,
    const RenderFloatImageView& source);

}  // namespace rb4
