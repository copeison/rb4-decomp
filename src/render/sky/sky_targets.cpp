#include "render/sky/sky_targets.h"

#include "render/textures/RndTextureBase.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"

namespace rb4 {

namespace {

std::uint32_t scaled_dimension(std::uint32_t value, std::uint32_t shift) {
    const auto scaled = value >> shift;
    return scaled > 0 ? scaled : 1;
}

RenderExtent scaled_extent(RenderExtent extent, std::uint32_t shift) {
    return {
        scaled_dimension(extent.width, shift),
        scaled_dimension(extent.height, shift),
    };
}

RndTextureBase*& target_slot(
    RenderTargetResources& resources,
    SkyTargetLevel level) {
    return resources.sky[static_cast<std::uint32_t>(level)];
}

RndTextureBase* target_slot(
    const RenderTargetResources& resources,
    SkyTargetLevel level) {
    return resources.sky[static_cast<std::uint32_t>(level)];
}

void release_target(
    RenderTargetResources& resources,
    SkyTargetLevel level) {
    auto*& target = target_slot(resources, level);
    if (target != nullptr) {
        delete target;
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B0E80.
void render_sky_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto extent = resources.extent;
    RndPixelFormat creation_state{};
    creation_state.mSettings[5] = 1;
    creation_state.mWrapMode = 1;
    creation_state.mFlags = 10;
    const RenderDataFormatDescriptor format_descriptor{
        32, 4, 1, 1, -1,
    };
    const auto data_format =
        render_data_format_resolve(format_descriptor, 7);
    RndTextureBase* new_full_target = nullptr;

    for (std::uint32_t shift = 0; shift < 4; ++shift) {
        const auto level = static_cast<SkyTargetLevel>(shift);
        RndTextureBase* reusable_target = nullptr;
        if (reusable_resources != nullptr) {
            reusable_target = target_slot(*reusable_resources, level);
        } else if (shift != 0) {
            reusable_target = new_full_target;
        }

        creation_state.mFilterMode = shift == 0 ? 1 : 2;

        auto* target = render_target_resources_create_texture_2d(
            resources,
            "Sky Buffer",
            creation_state,
            data_format,
            scaled_extent(extent, shift),
            -1,
            0,
            reusable_target);
        target_slot(resources, level) = target;
        if (shift == 0) {
            new_full_target = target;
        }
    }
}

// Reconstructed from the sky-target portion of eboot.elf at 0x6AFFE0.
void render_sky_targets_release(RenderTargetResources& resources) {
    release_target(resources, SkyTargetLevel::kFull);
    release_target(resources, SkyTargetLevel::kHalf);
    release_target(resources, SkyTargetLevel::kQuarter);
    release_target(resources, SkyTargetLevel::kEighth);
}

}  // namespace rb4
