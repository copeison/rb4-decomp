#include "render/intermediate/scaled_targets.h"

#include <array>

#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/intermediate/scaled_target_adapters.h"

namespace rb4 {

namespace {

constexpr std::array<ScaledTargetLevel, 3> kLevels{
    ScaledTargetLevel::kHalf,
    ScaledTargetLevel::kQuarter,
    ScaledTargetLevel::kEighth,
};

constexpr std::array<ScaledTargetLane, 2> kLanes{
    ScaledTargetLane::kFirst,
    ScaledTargetLane::kSecond,
};

std::uint32_t scaled_dimension(
    std::uint32_t value,
    ScaledTargetLevel level) {
    const auto scaled = value >> static_cast<std::uint32_t>(level);
    return scaled > 0 ? scaled : 1;
}

RenderExtent scaled_extent(RenderExtent extent, ScaledTargetLevel level) {
    return {
        scaled_dimension(extent.width, level),
        scaled_dimension(extent.height, level),
    };
}

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    ScaledTargetLevel level,
    ScaledTargetLane lane) {
    return resources == nullptr
        ? nullptr
        : render_target_resources_scaled_target(*resources, level, lane);
}

void release_target(
    RenderTargetResources& resources,
    ScaledTargetLevel level,
    ScaledTargetLane lane) {
    auto*& target =
        render_target_resources_scaled_target(resources, level, lane);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B12D0.
void render_scaled_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto full_extent = render_target_resources_extent(resources);

    for (const auto level : kLevels) {
        const auto extent = scaled_extent(full_extent, level);
        for (const auto lane : kLanes) {
            auto* target = render_target_resources_create_scaled_target(
                resources,
                level,
                extent,
                reusable_target(reusable_resources, level, lane));
            render_target_resources_scaled_target(resources, level, lane) =
                target;
        }
    }
}

// Reconstructed from the scaled-target portion of eboot.elf at 0x6AFFE0.
void render_scaled_targets_release(RenderTargetResources& resources) {
    for (const auto lane : kLanes) {
        for (const auto level : kLevels) {
            release_target(resources, level, lane);
        }
    }
}

}  // namespace rb4
