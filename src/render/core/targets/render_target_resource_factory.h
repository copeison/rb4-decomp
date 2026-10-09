#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner.h"
#include "render/textures/RndTextureBase.h"

namespace rb4 {

struct RenderTargetResources;
RndTextureBase* render_target_resources_create_texture_2d(
    RenderTargetResources& resources,
    const char* name,
    const RndPixelFormat& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RndTextureBase* reusable_texture);
RndTextureBase* render_target_resources_create_texture_array_2d(
    RenderTargetResources& resources,
    const char* name,
    const RndPixelFormat& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::size_t layer_count,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RndTextureBase* reusable_texture);

}  // namespace rb4
