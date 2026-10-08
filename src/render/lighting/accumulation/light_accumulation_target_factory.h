#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

RenderTexture* render_light_accumulation_target_create(
    RenderTargetResources& resources,
    const char* name,
    std::uint32_t scale_shift,
    bool allocate_attachment,
    RenderTexture* reusable_target);

}  // namespace rb4
