#pragma once

#include <cstdint>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTarget;

bool render_target_resources_force_64_bit_light_accumulation(
    const RenderTargetResources& resources);
std::uint32_t& render_target_resources_attachment_cursor(
    RenderTargetResources& resources);
RenderTarget* render_target_resources_create_light_accumulation_target_raw(
    RenderTargetResources& resources,
    const char* name,
    bool use_64_bit_format,
    RenderExtent extent,
    std::int32_t attachment_index,
    RenderTarget* reusable_target);
std::int32_t render_target_allocation_index(const RenderTarget& target);
std::int32_t render_target_allocation_count(const RenderTarget& target);

}  // namespace rb4
