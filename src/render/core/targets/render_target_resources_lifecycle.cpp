#include "render/core/targets/render_target_resources_lifecycle.h"

#include <cstddef>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/core/targets/render_target_resource_block.h"
#include "render/core/targets/render_target_resources_lifecycle_adapters.h"
#include "render/intermediate/scaled_targets.h"
#include "render/lighting/accumulation/light_accumulation_targets.h"
#include "render/lighting/probes/light_probe_accumulation_target.h"
#include "render/lighting/shadows/shadow_contribution_targets.h"
#include "render/masking/scene_mask_targets.h"
#include "render/masking/scene_mask_tiles.h"
#include "render/postprocessing/antialiasing/cmaa_targets.h"
#include "render/sky/sky_targets.h"

namespace rb4 {

namespace {

bool has_flag(
    std::uint32_t flags,
    RenderTargetResourceFlag flag) {
    return (flags & static_cast<std::uint32_t>(flag)) != 0;
}

const RenderTargetResourceBlock* reusable_block(
    const RenderTargetResources* resources,
    std::size_t index) {
    return resources == nullptr
        ? nullptr
        : &render_target_resources_block_at(*resources, index);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B0760.
void render_target_resources_initialize(
    RenderTargetResources& resources,
    RenderTexture& source_texture,
    const RenderTargetResources* reusable_resources) {
    render_target_resources_release(resources);
    render_target_resources_bind_source_texture(resources, source_texture);

    const auto flags = render_target_resources_flags(resources);
    if (has_flag(flags, RenderTargetResourceFlag::kLightAccumulation)) {
        render_light_accumulation_targets_create(
            resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kLightProbeAccumulation)) {
        render_light_probe_accumulation_target_create(
            resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kSky)) {
        render_sky_targets_create(resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kScaledTargets)) {
        render_scaled_targets_create(resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kSceneMask)) {
        render_scene_mask_targets_create(resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kShadowContribution)) {
        render_shadow_contribution_targets_create(
            resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kCmaa)) {
        render_cmaa_targets_create(resources, reusable_resources);
    }
    if (has_flag(flags, RenderTargetResourceFlag::kSceneMaskTiles)) {
        render_scene_mask_tiles_create(resources, reusable_resources);
    }

    render_target_resources_resize_blocks(resources, 1);
    render_target_resource_block_initialize(
        resources,
        render_target_resources_block_at(resources, 0),
        false,
        reusable_block(reusable_resources, 0));

    if (has_flag(flags, RenderTargetResourceFlag::kPartialFrameBlocks)) {
        const auto& settings =
            *render_system_settings(*render_system_instance());
        const auto partial_block_count = static_cast<std::size_t>(
            settings.max_partial_framerate_scenes);
        render_target_resources_resize_blocks(
            resources, partial_block_count + 1);
        for (std::size_t index = 1;
             index <= partial_block_count;
             ++index) {
            render_target_resource_block_initialize(
                resources,
                render_target_resources_block_at(resources, index),
                true,
                reusable_block(reusable_resources, index));
        }
    }

    render_target_resources_propagate_resource_mode(resources);
}

}  // namespace rb4
