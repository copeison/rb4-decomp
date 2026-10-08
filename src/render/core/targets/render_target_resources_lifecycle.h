#pragma once

#include <cstddef>
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

void render_target_resources_construct(
    RenderTargetResources& resources,
    std::uint32_t flags,
    std::int32_t resource_mode);
void render_target_resources_set_concrete_dispatch(
    RenderTargetResources& resources);
void render_target_resources_destruct(RenderTargetResources& resources);
void render_target_resources_initialize(
    RenderTargetResources& resources,
    RenderTexture& source_texture,
    const RenderTargetResources* reusable_resources);
void render_target_resources_release(RenderTargetResources& resources);
void render_target_resources_set_resource_mode(
    RenderTargetResources& resources,
    std::int32_t mode);
RenderPartialFrameState* render_target_resources_acquire_partial_frame_state(
    RenderTargetResources& resources,
    std::size_t partial_scene_index);
void render_target_resources_select_partial_frame(
    RenderTargetResources& resources,
    std::int64_t partial_scene_index,
    std::int64_t scene_context);

}  // namespace rb4
