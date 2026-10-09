#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

class RndTextureBase;

namespace rb4 {

RndTextureBase* render_light_accumulation_target_create(
    RenderTargetResources& resources,
    const char* name,
    std::uint32_t scale_shift,
    bool allocate_attachment,
    RndTextureBase* reusable_target);

}  // namespace rb4
