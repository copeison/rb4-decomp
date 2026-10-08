#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class VolumetricScatteringTextureKind : std::uint32_t {
    kInscattering,
    kStereoInscattering,
    kAccumulatedScattering,
};

enum class VolumetricScatteringDepth : std::uint32_t {
    k128 = 128,
    k256 = 256,
    k512 = 512,
};

struct RenderVolumeExtent {
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
};

void render_volumetric_scattering_textures_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block);
void render_volumetric_scattering_textures_release(
    RenderTargetResourceBlock& block);

}  // namespace rb4
