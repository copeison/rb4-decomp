#pragma once

#include <cstdint>
#include <vector>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/platform/render_platform.h"

namespace rb4 {

struct RenderPlatformConfig;

void render_platform_config_reset(RenderPlatformConfig& config);
void render_platform_config_apply_capabilities(
    RenderPlatformConfig& config,
    RenderPlatform platform);
std::vector<const char*> render_configured_resolutions(
    const char* platform_name);
void render_platform_config_set_resolutions(
    RenderPlatformConfig& config,
    const std::vector<RenderExtent>& resolutions);
std::vector<std::uint32_t> render_configured_supported_platform_ids(
    const char* block_name,
    const char* list_name);

}  // namespace rb4
