#pragma once

#include <cstdint>

#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderTextureCubeDescriptorState {
    RenderTextureMipChainDescriptor faces[6];
};

struct RenderTextureCubeState {
    RenderTextureMipChainState faces[6];
};

struct RenderTextureCubeDescriptor {
    RenderTextureDescriptorState texture_state;
    RenderTextureCubeDescriptorState cube;
};

struct RenderTextureCube : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureCubeState cube;
};

static_assert(sizeof(RenderTextureCubeDescriptorState) == 480);
static_assert(sizeof(RenderTextureCubeState) == 480);
static_assert(sizeof(RenderTextureCubeDescriptor) == 624);
static_assert(sizeof(RenderTextureCube) == 792);

void render_texture_cube_construct(
    RenderTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);
RenderTextureCube* render_create_texture_cube(
    RenderTextureCubeDescriptor& descriptor,
    RenderTextureCube* reusable_texture);
void render_texture_cube_destruct(RenderTextureCube& texture);
void render_texture_cube_delete(RenderTextureCube& texture);

}  // namespace rb4
