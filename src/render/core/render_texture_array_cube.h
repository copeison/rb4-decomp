#pragma once

#include <cstdint>

#include "render/core/render_texture.h"

namespace rb4 {

struct RenderTextureCubeDescriptorState {
    RenderTextureMipChainDescriptor faces[6];
};

struct RenderTextureCubeState {
    RenderTextureMipChainState faces[6];
};

struct RenderTextureCubeDescriptorRange {
    const RenderTextureCubeDescriptorState* begin;
    const RenderTextureCubeDescriptorState* end;
    const RenderTextureCubeDescriptorState* capacity;
};

struct RenderTextureArrayCubeDescriptor {
    std::uint8_t texture_state[144];
    RenderTextureCubeDescriptorRange cubes;
};

struct RenderTextureCubeArray {
    RenderTextureCubeState* begin;
    RenderTextureCubeState* end;
    RenderTextureCubeState* capacity;
    void* allocator;
};

struct RenderTextureArrayCube : RenderTexture {
    std::uint8_t descriptor_state[144];
    RenderTextureCubeArray cubes;
};

static_assert(sizeof(RenderTextureCubeDescriptorState) == 480);
static_assert(sizeof(RenderTextureCubeState) == 480);
static_assert(sizeof(RenderTextureCubeDescriptorRange) == 24);
static_assert(sizeof(RenderTextureArrayCubeDescriptor) == 168);
static_assert(sizeof(RenderTextureCubeArray) == 32);
static_assert(sizeof(RenderTextureArrayCube) == 344);

void render_texture_array_cube_construct(
    RenderTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);
void render_texture_array_cube_destruct(RenderTextureArrayCube& texture);
void render_texture_array_cube_delete(RenderTextureArrayCube& texture);

}  // namespace rb4
