#pragma once

#include "render/core/frame/render_frame_owner.h"
#include "render/postprocessing/antialiasing/cmaa_targets.h"

namespace rb4 {

struct RenderTarget;

bool render_system_supports_cmaa();
RenderTarget*& render_target_resources_cmaa_target(
    RenderTargetResources& resources,
    CmaaTargetKind kind);
RenderTarget* render_target_resources_cmaa_target(
    const RenderTargetResources& resources,
    CmaaTargetKind kind);
RenderTarget* render_target_resources_create_cmaa_target(
    RenderTargetResources& resources,
    CmaaTargetKind kind,
    RenderExtent extent,
    bool use_64_bit_color,
    RenderTarget* reusable_target);
void render_target_resources_reset_cmaa_state(
    RenderTargetResources& resources);

}  // namespace rb4
