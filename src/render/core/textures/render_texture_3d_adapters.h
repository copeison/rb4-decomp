#pragma once

#include "render/core/textures/render_texture_3d.h"

namespace rb4 {

void render_texture_3d_set_base_dispatch(RenderTexture3D& texture);
void render_texture_mip_chain_construct(
    RenderTextureMipChainState& mip_chain,
    const void* descriptor,
    bool has_source_data);
void render_texture_mip_chain_destruct(
    RenderTextureMipChainState& mip_chain);
void render_delete_texture_3d_storage(RenderTexture3D& texture);

}  // namespace rb4
