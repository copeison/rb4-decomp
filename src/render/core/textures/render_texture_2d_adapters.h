#pragma once

#include "render/core/textures/render_texture_2d.h"

namespace rb4 {

void render_texture_2d_set_base_dispatch(RenderTexture2D& texture);
void render_texture_mip_chain_construct(
    RenderTextureMipChainState& mip_chain,
    const void* descriptor,
    bool has_source_data);
void render_texture_mip_chain_destruct(
    RenderTextureMipChainState& mip_chain);
void render_delete_texture_2d_storage(RenderTexture2D& texture);

}  // namespace rb4
