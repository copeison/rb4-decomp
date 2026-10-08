#include "render/depth/linear_depth_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/depth/linear_depth_target_adapters.h"

namespace rb4 {

namespace {

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderTarget* reusable_target(
    const RenderTargetResourceBlock* block,
    LinearDepthTargetKind kind) {
    return block == nullptr
        ? nullptr
        : render_target_resource_block_linear_depth_target(*block, kind);
}

void create_target(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block,
    LinearDepthTargetKind kind,
    RenderExtent extent,
    bool register_with_owner) {
    render_target_resource_block_linear_depth_target(block, kind) =
        render_target_resources_create_linear_depth_target(
            resources,
            kind,
            extent,
            reusable_target(reusable_block, kind),
            register_with_owner);
}

void release_target(
    RenderTargetResourceBlock& block,
    LinearDepthTargetKind kind) {
    auto*& target =
        render_target_resource_block_linear_depth_target(block, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B2C90.
void render_linear_depth_targets_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block) {
    const auto extent = render_target_resources_extent(resources);
    create_target(
        resources,
        block,
        reusable_block,
        LinearDepthTargetKind::kLinearDepth,
        extent,
        !partial_frame);

    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (!settings.use_tiled_lighting) {
        return;
    }

    const auto tile_size =
        static_cast<std::uint32_t>(settings.light_tile_size);
    const RenderExtent tile_extent{
        divide_round_up(extent.width, tile_size),
        divide_round_up(extent.height, tile_size),
    };
    create_target(
        resources,
        block,
        reusable_block,
        LinearDepthTargetKind::kTiledDepthRange,
        tile_extent,
        !partial_frame);
}

// Reconstructed from the depth-target portion of eboot.elf at 0x6AFFE0.
void render_linear_depth_targets_release(RenderTargetResourceBlock& block) {
    release_target(block, LinearDepthTargetKind::kLinearDepth);
    release_target(block, LinearDepthTargetKind::kTiledDepthRange);
}

}  // namespace rb4
