#pragma once

#include "render/core/textures/render_texture_1d.h"

namespace rb4 {

void render_texture_1d_set_base_dispatch(RenderTexture1D& texture);
void render_texture_mip_chain_construct(
    RenderTextureMipChainState& mip_chain,
    const void* descriptor,
    bool has_source_data);
void render_texture_mip_chain_destruct(
    RenderTextureMipChainState& mip_chain);
void render_delete_texture_1d_storage(RenderTexture1D& texture);

}  // namespace rb4
