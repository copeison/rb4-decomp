#pragma once

#include <cstdint>

#include "render/lighting/volumetric/volumetric_scattering_textures.h"

namespace rb4 {

struct RenderTexture3D;

using VolumetricVoxelInitializer = std::uint64_t (*)(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z);

RenderTexture3D* render_target_resources_create_volumetric_scattering_texture(
    RenderTargetResources& resources,
    VolumetricScatteringTextureKind kind,
    RenderVolumeExtent extent,
    RenderTexture3D* reusable_texture,
    VolumetricVoxelInitializer initializer);

}  // namespace rb4
