#include "render/core/targets/render_target_resource_block.h"

#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/textures/render_texture.h"
#include "render/depth/depth_stencil_target.h"
#include "render/depth/linear_depth_targets.h"
#include "render/gbuffer/gbuffer_targets.h"
#include "render/lighting/accumulation/partial_light_accumulation_target.h"
#include "render/lighting/ambient_occlusion/ambient_occlusion_target.h"
#include "render/lighting/tiled/tiled_light_target_buffers.h"
#include "render/lighting/volumetric/volumetric_scattering_textures.h"

namespace rb4 {

static_assert(
    offsetof(RenderTargetResourceBlock, tiled_light_ids) ==
    offsetof(TiledLightTargetResources, light_ids));
static_assert(
    offsetof(RenderTargetResourceBlock, stereo_tiled_light_id_ranges) ==
    offsetof(TiledLightTargetResources, stereo_light_id_ranges));

TiledLightTargetResources& render_target_resource_block_tiled_light_resources(
    RenderTargetResourceBlock& block) {
    return reinterpret_cast<TiledLightTargetResources&>(block);
}

namespace {

enum class ResourceFlag : std::uint32_t {
    kDepthStencil = 0x0002,
    kLinearDepth = 0x0004,
    kLightAccumulation = 0x0008,
    kTiledLighting = 0x0020,
    kGBuffer = 0x0040,
    kAmbientOcclusion = 0x0800,
    kVolumetricScattering = 0x1000,
};

bool has_flag(std::uint32_t flags, ResourceFlag flag) {
    return (flags & static_cast<std::uint32_t>(flag)) != 0;
}

RenderExtent tiled_light_interpolation_extent(RenderExtent extent) {
    return {
        2 * (extent.width / 2 + (extent.width % 2 != 0)),
        extent.height / 2 + (extent.height % 2 != 0),
    };
}

RenderTexture* tiled_light_reuse_target(
    RenderTargetResources& resources,
    const RenderTargetResourceBlock* reusable_block,
    RenderExtent extent) {
    if (reusable_block != nullptr) {
        auto* target = reusable_block->tiled_light_interpolation;
        if (target != nullptr) {
            return target;
        }
    }
    auto* fallback = resources.shadow_contribution_scratch[0];
    if (fallback == nullptr) {
        return nullptr;
    }
    const auto interpolation_extent = tiled_light_interpolation_extent(extent);
    return fallback->width >= interpolation_extent.width &&
            fallback->height >= interpolation_extent.height
        ? fallback
        : nullptr;
}

RenderPartialFrameState* create_partial_frame_state() {
    auto* state = static_cast<RenderPartialFrameState*>(
        render_allocate(sizeof(RenderPartialFrameState)));
    *state = {};
    for (auto& value : state->values_00) {
        value = -1;
    }
    for (auto& value : state->values_18) {
        value = -1;
    }
    state->values_28[2] = 33;
    state->values_28[3] = 0xFFFFFFFFU;
    for (auto& value : state->values_40) {
        value = -1;
    }
    return state;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B2660.
void render_target_resource_block_initialize(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block) {
    const auto flags = resources.flags;
    if (partial_frame) {
        block.partial_frame_state = create_partial_frame_state();
        if (has_flag(flags, ResourceFlag::kLightAccumulation)) {
            render_partial_light_accumulation_target_create(
                resources, block, reusable_block);
        }
    }

    if (has_flag(flags, ResourceFlag::kDepthStencil)) {
        render_depth_stencil_target_create(
            resources, block, partial_frame, reusable_block);
    }
    if (has_flag(flags, ResourceFlag::kGBuffer)) {
        render_gbuffer_targets_create(
            resources, block, partial_frame, reusable_block);
    }
    if (has_flag(flags, ResourceFlag::kLinearDepth)) {
        render_linear_depth_targets_create(
            resources, block, partial_frame, reusable_block);
    }
    if (has_flag(flags, ResourceFlag::kAmbientOcclusion)) {
        render_ambient_occlusion_target_create(
            resources, block, partial_frame, reusable_block);
    }

    if (has_flag(flags, ResourceFlag::kTiledLighting)) {
        const auto extent = resources.extent;
        auto* interpolation_reuse = partial_frame
            ? nullptr
            : tiled_light_reuse_target(resources, reusable_block, extent);
        render_tiled_light_target_buffers_create(
            resources,
            render_target_resource_block_tiled_light_resources(block),
            extent,
            !partial_frame,
            resources.resource_mode == 3,
            interpolation_reuse);
    }

    if (has_flag(flags, ResourceFlag::kVolumetricScattering)) {
        render_volumetric_scattering_textures_create(
            resources, block, reusable_block);
    }
}

}  // namespace rb4
