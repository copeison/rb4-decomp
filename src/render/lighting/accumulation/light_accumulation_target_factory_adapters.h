#pragma once

#include <cstdint>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

bool render_target_resources_force_64_bit_light_accumulation(
    const RenderTargetResources& resources);
RenderTexture* render_target_resources_create_light_accumulation_target_raw(
    RenderTargetResources& resources,
    const char* name,
    bool use_64_bit_format,
    RenderExtent extent,
    std::int32_t attachment_index,
    RenderTexture* reusable_target);
std::int32_t render_target_allocation_index(const RenderTexture& target);
std::int32_t render_target_allocation_count(const RenderTexture& target);

}  // namespace rb4
