#include "render/core/system/render_system_lifecycle.h"

#include <cstddef>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_lifecycle_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kPlatformConfigCount = 13;
constexpr std::size_t kCurrentPlatformConfig = 7;

}  // namespace

// Reconstructed from eboot.elf at 0x3DD410.
void render_system_construct(RenderSystem& system) {
    render_system_construct_core_state(system);

    for (std::size_t index = 0; index < kPlatformConfigCount; ++index) {
        render_platform_config_construct(
            render_system_platform_config_at(system, index));
    }

    render_system_construct_default_resources(system);
    render_system_construct_backend_state(system);
    render_system_construct_callback_state(system);
    render_system_publish_instance(system);

    for (const auto platform_id : render_supported_platform_ids()) {
        if (platform_id < kPlatformConfigCount) {
            render_platform_config_initialize(
                render_system_platform_config_at(system, platform_id),
                platform_id);
        }
    }

    // The original invokes this predicate for platform slot seven and ignores
    // its result. Keep the call until its source-level purpose is known.
    render_platform_config_boot_probe(
        render_system_platform_config_at(system, kCurrentPlatformConfig));

    auto* settings = render_settings_allocate();
    render_settings_initialize(*settings);
    render_system_set_settings(system, settings);
}

// Reconstructed from eboot.elf at 0x3DD790.
void render_system_destruct(RenderSystem& system) {
    render_settings_release(render_system_settings(system));
    render_system_set_settings(system, nullptr);

    render_system_destroy_callback_state(system);
    render_system_destroy_backend_state(system);
    render_system_destroy_default_resources(system);

    for (std::size_t index = kPlatformConfigCount; index != 0; --index) {
        render_platform_config_destroy(
            render_system_platform_config_at(system, index - 1));
    }

    render_system_destroy_core_state(system);
}

}  // namespace rb4
