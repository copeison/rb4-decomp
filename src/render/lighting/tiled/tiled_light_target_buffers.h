#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner.h"

namespace rb4 {

struct RenderComputeBuffer;
struct RenderTargetResources;
struct RenderTexture;

struct TiledLightTargetResources {
    std::uint8_t reserved_0[88]{};
    RenderComputeBuffer* light_ids[2]{};
    RenderComputeBuffer* light_id_ranges = nullptr;
    RenderTexture* interpolation_target = nullptr;
    RenderComputeBuffer* stereo_light_ids[2]{};
    RenderComputeBuffer* stereo_light_id_ranges = nullptr;
};

static_assert(offsetof(TiledLightTargetResources, light_ids) == 88);
static_assert(offsetof(TiledLightTargetResources, light_id_ranges) == 104);
static_assert(offsetof(TiledLightTargetResources, interpolation_target) == 112);
static_assert(offsetof(TiledLightTargetResources, stereo_light_ids) == 120);
static_assert(
    offsetof(TiledLightTargetResources, stereo_light_id_ranges) == 136);
static_assert(sizeof(TiledLightTargetResources) == 144);

void render_tiled_light_target_buffers_create(
    RenderTargetResources& owner,
    TiledLightTargetResources& resources,
    RenderExtent extent,
    bool create_interpolation_target,
    bool stereo,
    RenderTexture* existing_interpolation_target);
void render_tiled_light_target_buffers_release(
    TiledLightTargetResources& resources);

}  // namespace rb4
