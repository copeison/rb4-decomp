#include "render/core/targets/render_target_resources_lifecycle.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_block.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_texture.h"
#include "render/depth/depth_stencil_target.h"
#include "render/depth/linear_depth_targets.h"
#include "render/gbuffer/gbuffer_targets.h"
#include "render/intermediate/scaled_targets.h"
#include "render/lighting/accumulation/partial_light_accumulation_target.h"
#include "render/lighting/accumulation/light_accumulation_targets.h"
#include "render/lighting/ambient_occlusion/ambient_occlusion_target.h"
#include "render/lighting/probes/light_probe_accumulation_target.h"
#include "render/lighting/shadows/shadow_contribution_targets.h"
#include "render/lighting/tiled/tiled_light_target_buffers.h"
#include "render/lighting/volumetric/volumetric_scattering_textures.h"
#include "render/masking/scene_mask_targets.h"
#include "render/masking/scene_mask_tiles.h"
#include "render/postprocessing/antialiasing/cmaa_targets.h"
#include "render/sky/sky_targets.h"

namespace rb4 {

namespace {

struct RenderTargetResourcesDispatch {
    void (*destruct)(RenderTargetResources& resources);
    void (*destroy)(RenderTargetResources& resources);
    bool (*accept_source_texture)(
        RenderTargetResources& resources,
        RenderTexture& source_texture);
    RenderTexture* (*create_texture_2d)(
        RenderTargetResources& resources,
        const char* name,
        const RenderTextureCreationState& creation_state,
        std::int32_t data_format,
        RenderExtent extent,
        std::int32_t attachment_index,
        std::uint32_t target_flags,
        RenderTexture* reusable_texture);
    RenderTexture* (*create_texture_array_2d)(
        RenderTargetResources& resources,
        const char* name,
        const RenderTextureCreationState& creation_state,
        std::int32_t data_format,
        RenderExtent extent,
        std::size_t layer_count,
        std::int32_t attachment_index,
        std::uint32_t target_flags,
        RenderTexture* reusable_texture);
};

struct RenderResourceModePrefix {
    std::uint8_t reserved_0[0xA0];
    std::int32_t resource_mode;
};

static_assert(offsetof(RenderResourceModePrefix, resource_mode) == 0xA0);
static_assert(sizeof(RenderTargetResourcesDispatch) == 5 * sizeof(void*));

void destroy_resources(RenderTargetResources& resources) {
    render_target_resources_destruct(resources);
    MemFree(&resources);
}

bool accept_2d_source_texture(
    RenderTargetResources&,
    RenderTexture& source_texture) {
    return render_texture_runtime_descriptor_type(source_texture) == 1;
}

RenderTargetResourcesDispatch kBaseDispatch{
    render_target_resources_destruct,
    destroy_resources,
    nullptr,
    nullptr,
    nullptr,
};

RenderTargetResourcesDispatch kConcreteDispatch{
    render_target_resources_destruct,
    destroy_resources,
    accept_2d_source_texture,
    render_target_resources_create_texture_2d,
    render_target_resources_create_texture_array_2d,
};

const RenderTargetResourcesDispatch& dispatch(
    const RenderTargetResources& resources) {
    return *static_cast<const RenderTargetResourcesDispatch*>(
        resources.implementation);
}

bool has_flag(
    std::uint32_t flags,
    RenderTargetResourceFlag flag) {
    return (flags & static_cast<std::uint32_t>(flag)) != 0;
}

void bind_source_texture(
    RenderTargetResources& resources,
    RenderTexture& source_texture) {
    resources.extent = {source_texture.width, source_texture.height};
    (void)dispatch(resources).accept_source_texture(
        resources, source_texture);
    resources.source_texture = &source_texture;
    resources.registered_resources_begin[
        resources.registered_resource_count++] = &source_texture;
}

void release_target(RenderTexture*& target) {
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

void resize_blocks(
    RenderTargetResources& resources,
    std::size_t count) {
    while (resources.block_count < count) {
        resources.blocks_begin[resources.block_count++] =
            RenderTargetResourceBlock{};
    }
    resources.block_count = count;
}

void propagate_resource_mode(RenderTargetResources& resources) {
    for (std::size_t index = 0;
         index < resources.registered_resource_count;
         ++index) {
        auto* resource = static_cast<RenderResourceModePrefix*>(
            resources.registered_resources_begin[index]);
        resource->resource_mode = resources.resource_mode;
    }
}

const RenderTargetResourceBlock* reusable_block(
    const RenderTargetResources* resources,
    std::size_t index) {
    return resources == nullptr
        ? nullptr
        : &resources->blocks_begin[index];
}

}  // namespace

void render_target_resources_set_concrete_dispatch(
    RenderTargetResources& resources) {
    resources.implementation = &kConcreteDispatch;
}

// Reconstructed from eboot.elf at 0x6AFEA0.
void render_target_resources_construct(
    RenderTargetResources& resources,
    std::uint32_t flags,
    std::int32_t resource_mode) {
    resources = RenderTargetResources{};
    resources.implementation = &kBaseDispatch;
    resources.flags = flags;
    resources.resource_mode = resource_mode;
    resources.registered_resources_begin =
        resources.registered_resources_inline;
    resources.registered_resource_capacity = 38;
    resources.blocks_begin = resources.blocks_inline;
    resources.block_capacity = 4;
    resources.active_scene_context = -1;
}

// Reconstructed from eboot.elf at 0x6AFFC0.
void render_target_resources_destruct(RenderTargetResources& resources) {
    resources.implementation = &kBaseDispatch;
    render_target_resources_release(resources);
}

// Reconstructed from eboot.elf at 0x6B0760.
void render_target_resources_initialize(
    RenderTargetResources& resources,
    RenderTexture& source_texture,
    const RenderTargetResources* reusable_resources) {
    render_target_resources_release(resources);
    bind_source_texture(resources, source_texture);

    const auto flags = resources.flags;
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

    resize_blocks(resources, 1);
    render_target_resource_block_initialize(
        resources,
        resources.blocks_begin[0],
        false,
        reusable_block(reusable_resources, 0));

    if (has_flag(flags, RenderTargetResourceFlag::kPartialFrameBlocks)) {
        const auto& settings =
            *render_system_settings(*render_system_instance());
        const auto partial_block_count = static_cast<std::size_t>(
            settings.max_partial_framerate_scenes);
        resize_blocks(resources, partial_block_count + 1);
        for (std::size_t index = 1;
             index <= partial_block_count;
             ++index) {
            render_target_resource_block_initialize(
                resources,
                resources.blocks_begin[index],
                true,
                reusable_block(reusable_resources, index));
        }
    }

    propagate_resource_mode(resources);
}

// Reconstructed from eboot.elf at 0x6AFFE0.
void render_target_resources_release(RenderTargetResources& resources) {
    const auto flags = resources.flags;
    auto*& source_texture = resources.source_texture;
    if (!has_flag(
            flags, RenderTargetResourceFlag::kSourceTextureNotOwned) &&
        source_texture != nullptr) {
        render_texture_release_dynamic(*source_texture);
    }
    source_texture = nullptr;

    render_light_accumulation_targets_release(resources);
    release_target(resources.unclassified_target_188);
    render_light_probe_accumulation_target_release(resources);
    render_sky_targets_release(resources);
    render_scaled_targets_release(resources);
    render_scene_mask_targets_release(resources);
    render_cmaa_targets_release(resources);
    render_shadow_contribution_targets_release(resources);
    render_scene_mask_tiles_release(resources);

    const auto block_count = resources.block_count;
    for (std::size_t index = 0; index < block_count; ++index) {
        auto& block = resources.blocks_begin[index];
        if (block.partial_frame_state != nullptr) {
            operator delete(block.partial_frame_state);
            block.partial_frame_state = nullptr;
        }
        render_partial_light_accumulation_target_release(block);
        render_depth_stencil_target_release(block);
        release_target(block.unclassified_target_18);
        release_target(block.unclassified_target_20);
        render_gbuffer_targets_release(block);
        render_linear_depth_targets_release(block);
        render_ambient_occlusion_target_release(block);
        render_tiled_light_target_buffers_release(
            render_target_resource_block_tiled_light_resources(block));
        render_volumetric_scattering_textures_release(block);
    }

    resources.block_count = 0;
    resources.extent = {};
    resources.attachment_cursor = 0;
    resources.registered_resource_count = 0;
}

// Reconstructed from eboot.elf at 0x6B28D0.
void render_target_resources_set_resource_mode(
    RenderTargetResources& resources,
    std::int32_t mode) {
    auto& current_mode = resources.resource_mode;
    if (current_mode != mode) {
        current_mode = mode;
        propagate_resource_mode(resources);
    }
}

// Reconstructed from eboot.elf at 0x6B2910.
RenderPartialFrameState* render_target_resources_acquire_partial_frame_state(
    RenderTargetResources& resources,
    std::size_t partial_scene_index) {
    const auto block_index = partial_scene_index + 1;
    const auto block_count = resources.block_count;
    if (block_index >= block_count) {
        const auto required_count = block_index + 1;
        resize_blocks(resources, required_count);
        for (auto index = block_count; index < required_count; ++index) {
            render_target_resource_block_initialize(
                resources,
                resources.blocks_begin[index],
                true,
                nullptr);
        }
    }
    return resources.blocks_begin[block_index].partial_frame_state;
}

// Reconstructed from eboot.elf at 0x6B2A20.
void render_target_resources_select_partial_frame(
    RenderTargetResources& resources,
    std::int64_t partial_scene_index,
    std::int64_t scene_context) {
    resources.active_block_index =
        static_cast<std::size_t>(partial_scene_index + 1);
    resources.active_scene_context = scene_context;
}

}  // namespace rb4
