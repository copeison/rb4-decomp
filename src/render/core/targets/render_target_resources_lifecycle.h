#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

enum class RenderTargetResourceFlag : std::uint32_t {
    kDepthStencil = 0x00000002,
    kLinearDepth = 0x00000004,
    kLightAccumulation = 0x00000008,
    kLightProbeAccumulation = 0x00000010,
    kTiledLighting = 0x00000020,
    kGBuffer = 0x00000040,
    kSky = 0x00000080,
    kScaledTargets = 0x00000100,
    kSceneMask = 0x00000200,
    kShadowContribution = 0x00000400,
    kAmbientOcclusion = 0x00000800,
    kVolumetricScattering = 0x00001000,
    kCmaa = 0x00002000,
    kSceneMaskTiles = 0x00004000,
    kForce64BitLightAccumulation = 0x20000000,
    kPartialFrameBlocks = 0x40000000,
    kSourceTextureNotOwned = 0x80000000,
};

void render_target_resources_initialize(
    RenderTargetResources& resources,
    RenderTexture& source_texture,
    const RenderTargetResources* reusable_resources);
void render_target_resources_release(RenderTargetResources& resources);

}  // namespace rb4
