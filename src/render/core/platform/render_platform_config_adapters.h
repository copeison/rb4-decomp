#pragma once

#include <cstdint>
#include <vector>

#include "render/core/frame/render_extent.h"
#include "render/core/platform/render_platform.h"

namespace rb4 {

struct RenderPlatformConfig;

std::vector<const char*> render_configured_resolutions(
    const char* platform_name);
std::vector<std::uint32_t> render_configured_supported_platform_ids(
    const char* block_name,
    const char* list_name);

}  // namespace rb4
