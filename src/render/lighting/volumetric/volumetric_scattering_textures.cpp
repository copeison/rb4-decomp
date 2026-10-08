#include "render/lighting/volumetric/volumetric_scattering_textures.h"

#include <array>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_3d.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/lighting/volumetric/volumetric_scattering_texture_adapters.h"

namespace rb4 {

namespace {

constexpr std::array<VolumetricScatteringDepth, 3> kDescendingDepths{
    VolumetricScatteringDepth::k512,
    VolumetricScatteringDepth::k256,
    VolumetricScatteringDepth::k128,
};

constexpr std::array<VolumetricScatteringDepth, 3> kAscendingDepths{
    VolumetricScatteringDepth::k128,
    VolumetricScatteringDepth::k256,
    VolumetricScatteringDepth::k512,
};

constexpr std::uint64_t kEvenAccumulatedScatteringVoxel =
    0x3C003C003C000000ULL;
constexpr std::uint64_t kOddAccumulatedScatteringVoxel =
    0x3C00000038003C00ULL;

std::uint32_t divide_round_up(
    std::uint32_t value,
    std::uint32_t divisor) {
    return value / divisor + (value % divisor != 0);
}

std::uint32_t align_up(std::uint32_t value, std::uint32_t alignment) {
    return divide_round_up(value, alignment) * alignment;
}

std::size_t depth_index(VolumetricScatteringDepth depth) {
    switch (depth) {
    case VolumetricScatteringDepth::k128:
        return 0;
    case VolumetricScatteringDepth::k256:
        return 1;
    case VolumetricScatteringDepth::k512:
        return 2;
    }
    return 0;
}

RenderTexture3D*& texture_slot(
    RenderTargetResourceBlock& block,
    VolumetricScatteringTextureKind kind,
    VolumetricScatteringDepth depth) {
    const auto index = depth_index(depth);
    switch (kind) {
    case VolumetricScatteringTextureKind::kInscattering:
        return block.volumetric_inscattering[index];
    case VolumetricScatteringTextureKind::kStereoInscattering:
        return block.stereo_volumetric_inscattering[index];
    case VolumetricScatteringTextureKind::kAccumulatedScattering:
        return block.accumulated_volumetric_scattering[index];
    }
    return block.volumetric_inscattering[index];
}

RenderTexture3D* texture_slot(
    const RenderTargetResourceBlock& block,
    VolumetricScatteringTextureKind kind,
    VolumetricScatteringDepth depth) {
    const auto index = depth_index(depth);
    switch (kind) {
    case VolumetricScatteringTextureKind::kInscattering:
        return block.volumetric_inscattering[index];
    case VolumetricScatteringTextureKind::kStereoInscattering:
        return block.stereo_volumetric_inscattering[index];
    case VolumetricScatteringTextureKind::kAccumulatedScattering:
        return block.accumulated_volumetric_scattering[index];
    }
    return nullptr;
}

std::uint64_t accumulated_scattering_voxel(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z) {
    return ((x ^ y ^ z) & 1U) == 0
        ? kEvenAccumulatedScatteringVoxel
        : kOddAccumulatedScatteringVoxel;
}

RenderTexture3D* matching_reusable_texture(
    const RenderTargetResourceBlock* block,
    VolumetricScatteringTextureKind kind,
    VolumetricScatteringDepth depth) {
    return block == nullptr
        ? nullptr
        : texture_slot(*block, kind, depth);
}

RenderTexture3D* first_creation_reuse_texture(
    RenderTargetResourceBlock& block,
    VolumetricScatteringTextureKind kind,
    VolumetricScatteringDepth depth) {
    if (kind == VolumetricScatteringTextureKind::kStereoInscattering ||
        depth == VolumetricScatteringDepth::k512) {
        return nullptr;
    }
    return texture_slot(block, kind, VolumetricScatteringDepth::k512);
}

void create_texture(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block,
    VolumetricScatteringTextureKind kind,
    VolumetricScatteringDepth depth,
    RenderExtent tile_extent) {
    auto* reusable_texture = reusable_block != nullptr
        ? matching_reusable_texture(reusable_block, kind, depth)
        : first_creation_reuse_texture(block, kind, depth);
    const RenderVolumeExtent volume_extent{
        tile_extent.width,
        tile_extent.height,
        static_cast<std::uint32_t>(depth),
    };
    const auto initializer =
        kind == VolumetricScatteringTextureKind::kAccumulatedScattering
        ? accumulated_scattering_voxel
        : nullptr;

    texture_slot(block, kind, depth) =
        render_target_resources_create_volumetric_scattering_texture(
            resources, kind, volume_extent, reusable_texture, initializer);
}

void release_texture(
    RenderTargetResourceBlock& block,
    VolumetricScatteringTextureKind kind,
    VolumetricScatteringDepth depth) {
    auto*& texture = texture_slot(block, kind, depth);
    if (texture != nullptr) {
        render_texture_release_dynamic(*texture);
        texture = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B3730.
void render_volumetric_scattering_textures_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (!settings.volumetric_scattering_enabled) {
        return;
    }

    const auto extent = resources.extent;
    const auto tile_size = static_cast<std::uint32_t>(
        settings.volumetric_scattering_tile_size);
    const RenderExtent tile_extent{
        align_up(divide_round_up(extent.width, tile_size), 8),
        align_up(divide_round_up(extent.height, tile_size), 8),
    };

    for (const auto depth : kDescendingDepths) {
        create_texture(
            resources,
            block,
            reusable_block,
            VolumetricScatteringTextureKind::kInscattering,
            depth,
            tile_extent);
        create_texture(
            resources,
            block,
            reusable_block,
            VolumetricScatteringTextureKind::kAccumulatedScattering,
            depth,
            tile_extent);
    }

    if (resources.resource_mode == 3) {
        for (const auto depth : kAscendingDepths) {
            create_texture(
                resources,
                block,
                reusable_block,
                VolumetricScatteringTextureKind::kStereoInscattering,
                depth,
                tile_extent);
        }
    }
}

// Reconstructed from the volumetric-scattering portion of eboot.elf at
// 0x6AFFE0.
void render_volumetric_scattering_textures_release(
    RenderTargetResourceBlock& block) {
    for (const auto depth : kAscendingDepths) {
        release_texture(
            block, VolumetricScatteringTextureKind::kInscattering, depth);
        release_texture(
            block,
            VolumetricScatteringTextureKind::kStereoInscattering,
            depth);
        release_texture(
            block,
            VolumetricScatteringTextureKind::kAccumulatedScattering,
            depth);
    }
}

}  // namespace rb4
