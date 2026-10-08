#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/masking/scene_mask_targets.h"

namespace rb4 {

struct RenderTarget;

RenderExtent render_target_resources_extent(
    const RenderTargetResources& resources);
RenderTarget*& render_target_resources_scene_mask_target(
    RenderTargetResources& resources,
    SceneMaskTargetKind kind);
RenderTarget* render_target_resources_scene_mask_target(
    const RenderTargetResources& resources,
    SceneMaskTargetKind kind);
RenderTarget* render_target_resources_create_scene_mask_target(
    RenderTargetResources& resources,
    SceneMaskTargetKind kind,
    RenderExtent extent,
    RenderTarget* reusable_target);

}  // namespace rb4
