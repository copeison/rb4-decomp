#pragma once

#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

bool render_texture_mip_chain_descriptor_copy_float_image(
    RenderTextureMipChainDescriptor& descriptor,
    const RenderFloatImageView& source);
}  // namespace rb4
