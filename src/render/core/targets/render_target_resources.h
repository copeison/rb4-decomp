#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner.h"

class RndComputeBuffer;

class RndTextureBase;
class RndTexture3D;

class RndMesh;

namespace rb4 {


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
    RndTextureBase* partial_light_accumulation;
    RndTextureBase* depth_stencil;
    RndTextureBase* unclassified_target_18;
    RndTextureBase* unclassified_target_20;
    RndTextureBase* gbuffer_color;
    RndTextureBase* gbuffer_pixel_normals;
    RndTextureBase* gbuffer_vertex_normals;
    RndTextureBase* linear_depth;
    RndTextureBase* tiled_depth_range;
    RndTextureBase* ambient_occlusion;
    RndComputeBuffer* tiled_light_ids[2];
    RndComputeBuffer* tiled_light_id_ranges;
    RndTextureBase* tiled_light_interpolation;
    RndComputeBuffer* stereo_tiled_light_ids[2];
    RndComputeBuffer* stereo_tiled_light_id_ranges;
    RndTexture3D* volumetric_inscattering[3];
    RndTexture3D* stereo_volumetric_inscattering[3];
    RndTexture3D* accumulated_volumetric_scattering[3];
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
    RndTextureBase* source_texture;
    RndTextureBase* light_accumulation[2];
    RndTextureBase* unclassified_target_188;
    RndTextureBase* blurred_light_accumulation[3];
    RndTextureBase* light_probe_accumulation;
    RndTextureBase* sky[4];
    RndTextureBase* scaled_targets[3][2];
    RndTextureBase* scene_mask;
    RndTextureBase* scene_mask_scratch;
    RndTextureBase* scene_mask_tiles;
    std::uint64_t cmaa_state;
    RndTextureBase* cmaa_color;
    RndTextureBase* cmaa_edges[2];
    RndTextureBase* cmaa_compressed_edges;
    RndTextureBase* shadow_contribution_texture_array;
    RndTextureBase* shadow_contribution_stencil;
    RndTextureBase* shadow_contribution_scratch[2];
    RndTextureBase* shadow_soften_tiles[2];
    RndTextureBase* tiled_scene_mask[2];
    RndMesh* tiled_scene_mask_mesh;

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
