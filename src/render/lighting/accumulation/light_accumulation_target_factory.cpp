#include "render/lighting/accumulation/light_accumulation_target_factory.h"

#include "render/core/settings/render_settings.h"
#include "render/system/RndDevice.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/textures/RndTextureBase.h"

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
RndTextureBase* render_light_accumulation_target_create(
    RenderTargetResources& resources,
    const char* name,
    std::uint32_t scale_shift,
    bool allocate_attachment,
    RndTextureBase* reusable_target) {
    const auto& settings =
        *TheRndDevice()->mSettings;
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
    RndPixelFormat creation_state{};
    creation_state.mSettings[5] = 1;
    creation_state.mWrapMode = static_cast<std::uint32_t>(
        TextureDefaultWrapMode(18));
    creation_state.mFilterMode = static_cast<std::uint32_t>(
        TextureDefaultFilterMode(18));
    creation_state.mFlags = 10;

    auto* target = render_target_resources_create_texture_2d(
        resources,
        name,
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        attachment_index,
        0,
        reusable_target);
    const auto allocation_index = target->mBaseDesc.mAttachmentIndex;
    if (allocation_index != -1) {
        resources.attachment_cursor =
            static_cast<std::uint32_t>(
                allocation_index + target->mBaseDesc.mAttachmentCount);
    }
    return target;
}

}  // namespace rb4
