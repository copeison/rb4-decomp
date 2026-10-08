#pragma once

#include <cstdint>
#include <vector>

#include "render/core/render_platform.h"

namespace rb4 {

struct RenderPlatformConfig;

void render_platform_config_construct(RenderPlatformConfig& config);
void render_platform_config_initialize(
    RenderPlatformConfig& config,
    std::uint32_t platform_id);
std::vector<std::uint32_t> render_supported_platform_ids();

}  // namespace rb4
