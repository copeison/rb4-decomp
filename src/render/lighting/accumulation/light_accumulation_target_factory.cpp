#include "render/lighting/accumulation/light_accumulation_target_factory.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/lighting/accumulation/light_accumulation_target_factory_adapters.h"

namespace rb4 {

namespace {

std::uint32_t scaled_dimension(std::uint32_t value, std::uint32_t shift) {
    for (std::uint32_t level = 0; level < shift; ++level) {
        value /= 2;
        if (value == 0) {
            value = 1;
        }
    }
    return value;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B2E80.
RenderTarget* render_light_accumulation_target_create(
    RenderTargetResources& resources,
    const char* name,
    std::uint32_t scale_shift,
    bool allocate_attachment,
    RenderTarget* reusable_target) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    const bool use_64_bit_format = settings.use_64_bit_light_accum ||
        render_target_resources_force_64_bit_light_accumulation(resources);

    const auto full_extent = resources.extent;
    const RenderExtent extent{
        scaled_dimension(full_extent.width, scale_shift),
        scaled_dimension(full_extent.height, scale_shift),
    };
    const auto attachment_index = allocate_attachment
        ? static_cast<std::int32_t>(
              resources.attachment_cursor)
        : -1;

    auto* target =
        render_target_resources_create_light_accumulation_target_raw(
            resources,
            name,
            use_64_bit_format,
            extent,
            attachment_index,
            reusable_target);
    const auto allocation_index = render_target_allocation_index(*target);
    if (allocation_index != -1) {
        resources.attachment_cursor =
            static_cast<std::uint32_t>(
                allocation_index + render_target_allocation_count(*target));
    }
    return target;
}

}  // namespace rb4
