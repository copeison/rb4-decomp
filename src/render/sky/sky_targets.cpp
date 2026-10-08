#include "render/sky/sky_targets.h"

#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/sky/sky_target_adapters.h"

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

void release_target(
    RenderTargetResources& resources,
    SkyTargetLevel level) {
    auto*& target = render_target_resources_sky_target(resources, level);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B0E80.
void render_sky_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto extent = resources.extent;
    RenderTarget* new_full_target = nullptr;

    for (std::uint32_t shift = 0; shift < 4; ++shift) {
        const auto level = static_cast<SkyTargetLevel>(shift);
        RenderTarget* reusable_target = nullptr;
        if (reusable_resources != nullptr) {
            reusable_target =
                render_target_resources_sky_target(*reusable_resources, level);
        } else if (shift != 0) {
            reusable_target = new_full_target;
        }

        auto* target = render_target_resources_create_sky_target(
            resources,
            level,
            scaled_extent(extent, shift),
            reusable_target);
        render_target_resources_sky_target(resources, level) = target;
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
