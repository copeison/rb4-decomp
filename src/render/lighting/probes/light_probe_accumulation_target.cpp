#include "render/lighting/probes/light_probe_accumulation_target.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
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
        : reusable_resources->light_probe_accumulation;
    resources.light_probe_accumulation =
        render_target_resources_create_light_probe_accumulation_target(
            resources, reusable_target);
}

// Reconstructed from the light-probe portion of eboot.elf at 0x6AFFE0.
void render_light_probe_accumulation_target_release(
    RenderTargetResources& resources) {
    auto*& target = resources.light_probe_accumulation;
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
