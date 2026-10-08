#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rb4 {

struct RenderPlatformConfig;
struct RenderSettings;
struct RenderSystem;

void render_system_install_base_vtable(RenderSystem& system);

RenderPlatformConfig& render_system_platform_config_at(
    RenderSystem& system,
    std::size_t index);
void render_platform_config_construct(RenderPlatformConfig& config);
void render_platform_config_initialize(
    RenderPlatformConfig& config,
    std::uint32_t platform_id);
void render_platform_config_destroy(RenderPlatformConfig& config);
bool render_platform_config_boot_probe(const RenderPlatformConfig& config);
std::vector<std::uint32_t> render_supported_platform_ids();

void render_system_construct_default_resources(RenderSystem& system);
void render_system_destroy_default_resources(RenderSystem& system);
void render_system_construct_backend_state(RenderSystem& system);
void render_system_destroy_backend_state(RenderSystem& system);
void render_system_construct_callback_state(RenderSystem& system);
void render_system_destroy_callback_state(RenderSystem& system);

RenderSettings* render_settings_allocate();
void render_settings_release(RenderSettings* settings);
}  // namespace rb4
