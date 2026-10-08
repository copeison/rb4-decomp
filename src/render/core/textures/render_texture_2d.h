#pragma once

#include <cstdint>

#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderTexture2DDescriptor {
    std::uint8_t texture_state[144];
    RenderTextureMipChainDescriptor mip_chain;
};

struct RenderTexture2D : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureMipChainState mip_chain;
    void* linked_resource;
    std::int64_t linked_resource_index;
};

static_assert(sizeof(RenderTexture2DDescriptor) == 224);
static_assert(sizeof(RenderTexture2D) == 408);

void render_texture_2d_construct(
    RenderTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor);
void render_texture_2d_destruct(RenderTexture2D& texture);
void render_texture_2d_delete(RenderTexture2D& texture);
void render_texture_2d_set_linked_resource(
    RenderTexture2D& texture,
    void* resource,
    std::int64_t resource_index);

}  // namespace rb4
