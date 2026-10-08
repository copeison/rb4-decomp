#pragma once

#include <cstdint>

#include "render/depth/depth_stencil_target.h"

namespace rb4 {

struct RenderTexture;

std::int32_t& render_target_resources_depth_attachment_end(
    RenderTargetResources& resources);
bool render_depth_stencil_target_has_unassigned_attachment(
    const RenderTexture& target);
void render_target_resources_update_depth_attachment_end(
    RenderTargetResources& resources,
    const RenderTexture& target);
RenderTexture* render_target_resources_create_depth_stencil_target(
    RenderTargetResources& resources,
    bool use_40_bit_depth_stencil,
    std::int32_t attachment_index,
    RenderTexture* reusable_target,
    bool register_with_owner);

}  // namespace rb4
