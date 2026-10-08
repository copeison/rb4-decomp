#include "render/intermediate/scaled_targets.h"

#include <array>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture_adapters.h"

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

const char* target_name(ScaledTargetLevel level) {
    switch (level) {
        case ScaledTargetLevel::kHalf:
            return "Half-Size Buffer";
        case ScaledTargetLevel::kQuarter:
            return "Quarter-Size Buffer";
        case ScaledTargetLevel::kEighth:
            return "Eighth-Size Buffer";
    }
    return "Scaled Buffer";
}

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

RenderTexture*& target_slot(
    RenderTargetResources& resources,
    ScaledTargetLevel level,
    ScaledTargetLane lane) {
    const auto level_index = static_cast<std::uint32_t>(level) - 1;
    return resources.scaled_targets[level_index]
                                   [static_cast<std::uint32_t>(lane)];
}

RenderTexture* target_slot(
    const RenderTargetResources& resources,
    ScaledTargetLevel level,
    ScaledTargetLane lane) {
    const auto level_index = static_cast<std::uint32_t>(level) - 1;
    return resources.scaled_targets[level_index]
                                   [static_cast<std::uint32_t>(lane)];
}

RenderTexture* reusable_target(
    const RenderTargetResources* resources,
    ScaledTargetLevel level,
    ScaledTargetLane lane) {
    return resources == nullptr
        ? nullptr
        : target_slot(*resources, level, lane);
}

void release_target(
    RenderTargetResources& resources,
    ScaledTargetLevel level,
    ScaledTargetLane lane) {
    auto*& target = target_slot(resources, level, lane);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B12D0.
void render_scaled_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    const auto full_extent = resources.extent;
    const auto use_64_bit_format =
        render_system_settings(*render_system_instance())
            ->use_64_bit_light_accum;
    const RenderDataFormatDescriptor format_descriptor{
        use_64_bit_format ? 64U : 32U,
        use_64_bit_format ? 4U : 2U,
        2,
        1,
        -1,
    };
    const auto data_format =
        render_data_format_resolve(format_descriptor, 7);
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = 1;
    creation_state.values[9] = 2;
    creation_state.values[10] = 10;

    for (const auto level : kLevels) {
        const auto extent = scaled_extent(full_extent, level);
        for (const auto lane : kLanes) {
            auto* target = render_target_resources_create_texture_2d(
                resources,
                target_name(level),
                creation_state,
                data_format,
                extent,
                -1,
                0,
                reusable_target(reusable_resources, level, lane));
            target_slot(resources, level, lane) = target;
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
