#include "render/lighting/probes/light_probe_accumulation_target.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/lighting/probes/light_probe_accumulation_target_adapters.h"

namespace rb4 {

// Reconstructed from the light-probe portion of eboot.elf at 0x6B07DC.
void render_light_probe_accumulation_target_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (settings.volumetric_scattering_enabled) {
        return;
    }

    auto* reusable_target = reusable_resources == nullptr
        ? nullptr
        : render_target_resources_light_probe_accumulation_target(
              *reusable_resources);
    render_target_resources_light_probe_accumulation_target(resources) =
        render_target_resources_create_light_probe_accumulation_target(
            resources, reusable_target);
}

// Reconstructed from the light-probe portion of eboot.elf at 0x6AFFE0.
void render_light_probe_accumulation_target_release(
    RenderTargetResources& resources) {
    auto*& target =
        render_target_resources_light_probe_accumulation_target(resources);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
