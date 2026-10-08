#pragma once

#include <cstddef>

#include "render/core/textures/render_texture_array_2d.h"

namespace rb4 {

void render_texture_array_2d_set_base_dispatch(
    RenderTextureArray2D& texture);
void render_texture_mip_chain_array_construct(
    RenderTextureMipChainArray& mip_chains);
void render_texture_mip_chain_array_reserve(
    RenderTextureMipChainArray& mip_chains,
    std::size_t capacity);
void render_texture_mip_chain_array_append(
    RenderTextureMipChainArray& mip_chains,
    const RenderTextureMipChainDescriptor& descriptor,
    bool has_source_data);
void render_texture_array_2d_resolve_descriptor(
    RenderTextureDescriptorState& descriptor_state);
void render_texture_mip_chain_array_destruct(
    RenderTextureMipChainArray& mip_chains);
void render_delete_texture_array_2d_storage(
    RenderTextureArray2D& texture);

}  // namespace rb4
