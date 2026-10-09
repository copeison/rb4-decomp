#include "render/depth/linear_depth_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/system/RndDevice.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/textures/RndTextureBase.h"

namespace rb4 {

namespace {

RndTextureBase*& target_slot(
    RenderTargetResourceBlock& block,
    LinearDepthTargetKind kind) {
    return kind == LinearDepthTargetKind::kLinearDepth
        ? block.linear_depth
        : block.tiled_depth_range;
}

RndTextureBase* target_slot(
    const RenderTargetResourceBlock& block,
    LinearDepthTargetKind kind) {
    return kind == LinearDepthTargetKind::kLinearDepth
        ? block.linear_depth
        : block.tiled_depth_range;
}

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RndTextureBase* reusable_target(
    const RenderTargetResourceBlock* block,
    LinearDepthTargetKind kind) {
    return block == nullptr
        ? nullptr
        : target_slot(*block, kind);
}

void create_target(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block,
    LinearDepthTargetKind kind,
    RenderExtent extent,
    bool register_with_owner) {
    RndPixelFormat creation_state{};
    creation_state.mSettings[5] = 1;
    creation_state.mWrapMode = static_cast<std::uint32_t>(
        TextureDefaultWrapMode(7));
    creation_state.mFilterMode = static_cast<std::uint32_t>(
        TextureDefaultFilterMode(7));
    creation_state.mFlags = 10;
    const bool tiled = kind == LinearDepthTargetKind::kTiledDepthRange;
    const RenderDataFormatDescriptor format_descriptor{
        tiled ? 32U : 16U,
        tiled ? 0U : 10U,
        0,
        1,
        -1,
    };
    auto* target = render_target_resources_create_texture_2d(
        resources,
        tiled ? "Tiled Depth Range" : "Linear Depth Buffer",
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        -1,
        0,
        reusable_target(reusable_block, kind));
    target_slot(block, kind) = target;
    if (register_with_owner) {
        resources.registered_resources_begin[
            resources.registered_resource_count++] = target;
    }
}

void release_target(
    RenderTargetResourceBlock& block,
    LinearDepthTargetKind kind) {
    auto*& target = target_slot(block, kind);
    if (target != nullptr) {
        delete target;
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
    const auto extent = resources.extent;
    create_target(
        resources,
        block,
        reusable_block,
        LinearDepthTargetKind::kLinearDepth,
        extent,
        !partial_frame);

    const auto& settings =
        *TheRndDevice()->mSettings;
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
