#include "render/lighting/probes/light_probe_accumulation_target.h"

#include "render/core/textures/render_texture.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"

namespace rb4 {

// Reconstructed from the light-probe portion of eboot.elf at 0x6B07DC.
void render_light_probe_accumulation_target_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (settings.use_tiled_lighting) {
        return;
    }

    auto* reusable_target = reusable_resources == nullptr
        ? nullptr
        : reusable_resources->light_probe_accumulation;
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = 1;
    creation_state.values[9] = 1;
    creation_state.values[10] = 10;
    const RenderDataFormatDescriptor format_descriptor{
        64, 4, 2, 1, -1,
    };
    auto* target = render_target_resources_create_texture_2d(
        resources,
        "Light Probe Accum Buffer",
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        resources.extent,
        -1,
        0,
        reusable_target);
    resources.light_probe_accumulation = target;
    resources.registered_resources_begin[
        resources.registered_resource_count++] = target;
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
