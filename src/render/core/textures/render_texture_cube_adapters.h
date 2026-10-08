#pragma once

#include "render/core/textures/render_texture_cube.h"

namespace rb4 {

void render_texture_cube_set_base_dispatch(RenderTextureCube& texture);
void render_texture_cube_prepare_descriptor(
    RenderTextureCubeDescriptorState& cube);
void render_texture_cube_state_construct(
    RenderTextureCubeState& cube,
    const RenderTextureCubeDescriptorState& descriptor,
    bool has_source_data);
void render_texture_cube_state_destruct(RenderTextureCubeState& cube);
void render_delete_texture_cube_storage(RenderTextureCube& texture);

}  // namespace rb4
