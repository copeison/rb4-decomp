#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner.h"

class RndComputeBuffer;

namespace rb4 {

struct RenderMesh;
struct RenderTexture;
struct RenderTexture3D;

struct RenderPartialFrameState {
    std::int32_t values_00[5];
    std::uint32_t reserved_14;
    std::int32_t values_18[4];
    std::uint32_t values_28[4];
    std::uint32_t value_38;
    bool value_3C;
    std::uint8_t reserved_3D[3];
    std::int32_t values_40[3];
    std::uint16_t value_4C;
    bool value_4E;
    std::uint8_t reserved_4F;
};

struct RenderTargetResourceBlock {
    RenderPartialFrameState* partial_frame_state;
    RenderTexture* partial_light_accumulation;
    RenderTexture* depth_stencil;
    RenderTexture* unclassified_target_18;
    RenderTexture* unclassified_target_20;
    RenderTexture* gbuffer_color;
    RenderTexture* gbuffer_pixel_normals;
    RenderTexture* gbuffer_vertex_normals;
    RenderTexture* linear_depth;
    RenderTexture* tiled_depth_range;
    RenderTexture* ambient_occlusion;
    RndComputeBuffer* tiled_light_ids[2];
    RndComputeBuffer* tiled_light_id_ranges;
    RenderTexture* tiled_light_interpolation;
    RndComputeBuffer* stereo_tiled_light_ids[2];
    RndComputeBuffer* stereo_tiled_light_id_ranges;
    RenderTexture3D* volumetric_inscattering[3];
    RenderTexture3D* stereo_volumetric_inscattering[3];
    RenderTexture3D* accumulated_volumetric_scattering[3];
};

struct RenderTargetResources {
    void* implementation;
    std::uint32_t flags;
    std::int32_t resource_mode;
    std::uint64_t reserved_10;
    RenderExtent extent;

    void** registered_resources_begin;
    std::size_t registered_resource_count;
    std::size_t registered_resource_capacity;
    void* registered_resources_inline[38];

    std::uint64_t attachment_cursor;
    RenderTexture* source_texture;
    RenderTexture* light_accumulation[2];
    RenderTexture* unclassified_target_188;
    RenderTexture* blurred_light_accumulation[3];
    RenderTexture* light_probe_accumulation;
    RenderTexture* sky[4];
    RenderTexture* scaled_targets[3][2];
    RenderTexture* scene_mask;
    RenderTexture* scene_mask_scratch;
    RenderTexture* scene_mask_tiles;
    std::uint64_t cmaa_state;
    RenderTexture* cmaa_color;
    RenderTexture* cmaa_edges[2];
    RenderTexture* cmaa_compressed_edges;
    RenderTexture* shadow_contribution_texture_array;
    RenderTexture* shadow_contribution_stencil;
    RenderTexture* shadow_contribution_scratch[2];
    RenderTexture* shadow_soften_tiles[2];
    RenderTexture* tiled_scene_mask[2];
    RenderMesh* tiled_scene_mask_mesh;

    RenderTargetResourceBlock* blocks_begin;
    std::size_t block_count;
    std::size_t block_capacity;
    RenderTargetResourceBlock blocks_inline[4];
    std::size_t active_block_index;
    std::int64_t active_scene_context;
};

static_assert(sizeof(RenderPartialFrameState) == 80);
static_assert(offsetof(RenderPartialFrameState, values_18) == 0x18);
static_assert(offsetof(RenderPartialFrameState, values_28) == 0x28);
static_assert(offsetof(RenderPartialFrameState, value_38) == 0x38);
static_assert(offsetof(RenderPartialFrameState, values_40) == 0x40);
static_assert(sizeof(RenderTargetResourceBlock) == 216);
static_assert(offsetof(RenderTargetResourceBlock, depth_stencil) == 0x10);
static_assert(offsetof(RenderTargetResourceBlock, gbuffer_color) == 0x28);
static_assert(offsetof(RenderTargetResourceBlock, linear_depth) == 0x40);
static_assert(offsetof(RenderTargetResourceBlock, ambient_occlusion) == 0x50);
static_assert(offsetof(RenderTargetResourceBlock, tiled_light_ids) == 0x58);
static_assert(
    offsetof(RenderTargetResourceBlock, volumetric_inscattering) == 0x90);
static_assert(
    offsetof(RenderTargetResourceBlock, accumulated_volumetric_scattering) ==
    0xC0);

static_assert(offsetof(RenderTargetResources, extent) == 0x18);
static_assert(
    offsetof(RenderTargetResources, registered_resources_inline) == 0x38);
static_assert(offsetof(RenderTargetResources, attachment_cursor) == 0x168);
static_assert(offsetof(RenderTargetResources, source_texture) == 0x170);
static_assert(offsetof(RenderTargetResources, light_accumulation) == 0x178);
static_assert(offsetof(RenderTargetResources, sky) == 0x1B0);
static_assert(offsetof(RenderTargetResources, scaled_targets) == 0x1D0);
static_assert(offsetof(RenderTargetResources, scene_mask) == 0x200);
static_assert(offsetof(RenderTargetResources, cmaa_state) == 0x218);
static_assert(
    offsetof(RenderTargetResources, shadow_contribution_texture_array) ==
    0x240);
static_assert(offsetof(RenderTargetResources, tiled_scene_mask) == 0x270);
static_assert(offsetof(RenderTargetResources, blocks_begin) == 0x288);
static_assert(offsetof(RenderTargetResources, blocks_inline) == 0x2A0);
static_assert(offsetof(RenderTargetResources, active_block_index) == 0x600);
static_assert(offsetof(RenderTargetResources, active_scene_context) == 0x608);
static_assert(sizeof(RenderTargetResources) == 1552);

}  // namespace rb4
