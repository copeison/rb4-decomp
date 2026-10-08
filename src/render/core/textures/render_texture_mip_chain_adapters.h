#pragma once

#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

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
void render_texture_mip_chain_construct(
    RenderTextureMipChainState& mip_chain,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data);
void render_texture_mip_chain_destruct(
    RenderTextureMipChainState& mip_chain);
}  // namespace rb4
