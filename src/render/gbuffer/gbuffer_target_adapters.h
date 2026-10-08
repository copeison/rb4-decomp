#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/gbuffer/gbuffer_targets.h"

namespace rb4 {

struct RenderTarget;

RenderTarget*& render_target_resource_block_gbuffer_target(
    RenderTargetResourceBlock& block,
    GBufferTargetKind kind);
RenderTarget* render_target_resource_block_gbuffer_target(
    const RenderTargetResourceBlock& block,
    GBufferTargetKind kind);
RenderTarget* render_target_resources_create_gbuffer_target(
    RenderTargetResources& resources,
    GBufferTargetKind kind,
    RenderExtent extent,
    RenderTarget* reusable_target,
    bool register_with_owner);

}  // namespace rb4
