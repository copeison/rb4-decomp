#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/textures/render_texture.h"

namespace rb4 {

struct RenderTargetResources;
RenderTexture* render_target_resources_create_texture_2d(
    RenderTargetResources& resources,
    const char* name,
    const RenderTextureCreationState& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RenderTexture* reusable_texture);
RenderTexture* render_target_resources_create_texture_array_2d(
    RenderTargetResources& resources,
    const char* name,
    const RenderTextureCreationState& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::size_t layer_count,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RenderTexture* reusable_texture);

}  // namespace rb4
