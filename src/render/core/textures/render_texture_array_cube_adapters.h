#pragma once

#include "render/core/textures/render_texture_array_cube.h"

namespace rb4 {

void render_texture_array_cube_set_base_dispatch(
    RenderTextureArrayCube& texture);
void render_texture_cube_array_construct(
    RenderTextureDescriptorState& descriptor_state,
    RenderTextureCubeArray& cubes,
    const RenderTextureArrayCubeDescriptor& descriptor,
    bool has_source_data);
void render_texture_cube_array_validate(
    const RenderTextureCubeArray& cubes);
void render_texture_cube_array_destruct(RenderTextureCubeArray& cubes);
void render_delete_texture_array_cube_storage(
    RenderTextureArrayCube& texture);

}  // namespace rb4
