#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/platform/render_platform.h"

namespace rb4 {

struct RenderSystem;

struct RenderPlatformConfig {
    std::vector<RenderExtent> resolutions;
    std::uint64_t resource_tier;
    std::uint32_t feature_flags;
    std::uint32_t reserved_44;
    std::uint64_t capability_values[5];
    bool enabled;
    std::uint8_t reserved_89[7];
    std::uint64_t resource_budget;
    std::uint32_t option_flags;
    std::uint32_t reserved_108;
    std::uint64_t capability_mask[2];
};

static_assert(offsetof(RenderPlatformConfig, resource_tier) == 32);
static_assert(offsetof(RenderPlatformConfig, feature_flags) == 40);
static_assert(offsetof(RenderPlatformConfig, enabled) == 88);
static_assert(offsetof(RenderPlatformConfig, resource_budget) == 96);
static_assert(offsetof(RenderPlatformConfig, capability_mask) == 112);
static_assert(sizeof(RenderPlatformConfig) == 128);

RenderPlatformConfig& render_system_platform_config_at(
    RenderSystem& system,
    std::size_t index);
void render_platform_config_construct(RenderPlatformConfig& config);
void render_platform_config_initialize(
    RenderPlatformConfig& config,
    std::uint32_t platform_id);
void render_platform_config_destruct(RenderPlatformConfig& config);
bool render_platform_config_boot_probe(const RenderPlatformConfig& config);
std::vector<std::uint32_t> render_supported_platform_ids();

}  // namespace rb4
