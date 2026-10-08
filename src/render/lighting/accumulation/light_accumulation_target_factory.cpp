#include "render/lighting/accumulation/light_accumulation_target_factory.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format_adapters.h"
#include "render/core/textures/render_texture.h"

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
RenderTexture* render_light_accumulation_target_create(
    RenderTargetResources& resources,
    const char* name,
    std::uint32_t scale_shift,
    bool allocate_attachment,
    RenderTexture* reusable_target) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    const bool use_64_bit_format = settings.use_64_bit_light_accum ||
        (resources.flags & 0x20000000U) != 0;

    const auto full_extent = resources.extent;
    const RenderExtent extent{
        scaled_dimension(full_extent.width, scale_shift),
        scaled_dimension(full_extent.height, scale_shift),
    };
    const auto attachment_index = allocate_attachment
        ? static_cast<std::int32_t>(
              resources.attachment_cursor)
        : -1;

    const RenderDataFormatDescriptor format_descriptor{
        use_64_bit_format ? 64U : 32U,
        use_64_bit_format ? 4U : 2U,
        2,
        1,
        -1,
    };
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = static_cast<std::uint32_t>(
        render_texture_default_address_mode(18));
    creation_state.values[9] = static_cast<std::uint32_t>(
        render_texture_default_filter_mode(18));
    creation_state.values[10] = 10;

    auto* target = render_target_resources_create_texture_2d(
        resources,
        name,
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        attachment_index,
        0,
        reusable_target);
    const auto allocation_index = target->attachment_index;
    if (allocation_index != -1) {
        resources.attachment_cursor =
            static_cast<std::uint32_t>(
                allocation_index + target->attachment_count);
    }
    return target;
}

}  // namespace rb4
