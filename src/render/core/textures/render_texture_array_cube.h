#pragma once

#include <cstdint>

#include "render/core/textures/render_texture_cube.h"

namespace rb4 {

struct RenderTextureCubeDescriptorRange {
    const RenderTextureCubeDescriptorState* begin;
    const RenderTextureCubeDescriptorState* end;
    const RenderTextureCubeDescriptorState* capacity;
};

struct RenderTextureArrayCubeDescriptor {
    RenderTextureDescriptorState texture_state;
    RenderTextureCubeDescriptorRange cubes;
};

struct RenderTextureCubeArray {
    RenderTextureCubeState* begin;
    RenderTextureCubeState* end;
    RenderTextureCubeState* capacity;
    void* allocator;
};

struct RenderTextureArrayCube : RenderTexture {
    RenderTextureDescriptorState descriptor_state;
    RenderTextureCubeArray cubes;
};

void render_texture_cube_array_construct(
    RenderTextureDescriptorState& descriptor_state,
    RenderTextureCubeArray& cubes,
    const RenderTextureArrayCubeDescriptor& descriptor,
    bool has_source_data);
bool render_texture_cube_array_validate(
    const RenderTextureCubeArray& cubes);
void render_texture_cube_array_destruct(RenderTextureCubeArray& cubes);

static_assert(sizeof(RenderTextureCubeDescriptorRange) == 24);
static_assert(sizeof(RenderTextureArrayCubeDescriptor) == 168);
static_assert(sizeof(RenderTextureCubeArray) == 32);
static_assert(sizeof(RenderTextureArrayCube) == 344);

void render_texture_array_cube_descriptor_construct(
    RenderTextureArrayCubeDescriptor& descriptor);
void render_texture_array_cube_construct(
    RenderTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);
RenderTextureArrayCube* render_create_texture_array_cube(
    RenderTextureArrayCubeDescriptor& descriptor,
    RenderTextureArrayCube* reusable_texture);
void render_texture_array_cube_destruct(RenderTextureArrayCube& texture);
void render_texture_array_cube_delete(RenderTextureArrayCube& texture);

}  // namespace rb4
