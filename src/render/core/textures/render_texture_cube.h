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
    RenderTextureDescriptorState descriptor_state;
    RenderTextureCubeState cube;
};

bool render_texture_cube_prepare_descriptor(
    const RenderTextureCubeDescriptorState& cube);
void render_texture_cube_state_construct(
    RenderTextureCubeState& cube,
    const RenderTextureCubeDescriptorState& descriptor,
    bool has_source_data);
void render_texture_cube_state_destruct(RenderTextureCubeState& cube);
std::size_t render_texture_cube_state_source_size(
    const RenderTextureCubeState& cube);
void render_texture_cube_state_release_source_data(
    RenderTextureCubeState& cube);

static_assert(sizeof(RenderTextureCubeDescriptorState) == 480);
static_assert(sizeof(RenderTextureCubeState) == 480);
static_assert(sizeof(RenderTextureCubeDescriptor) == 624);
static_assert(sizeof(RenderTextureCube) == 792);

void render_texture_cube_descriptor_construct(
    RenderTextureCubeDescriptor& descriptor);
void render_texture_cube_construct(
    RenderTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);
RenderTextureCube* render_create_texture_cube(
    RenderTextureCubeDescriptor& descriptor,
    RenderTextureCube* reusable_texture);
void render_texture_cube_destruct(RenderTextureCube& texture);
void render_texture_cube_delete(RenderTextureCube& texture);

}  // namespace rb4
