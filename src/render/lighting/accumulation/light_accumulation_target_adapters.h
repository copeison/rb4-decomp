#pragma once

#include "render/lighting/accumulation/light_accumulation_targets.h"

namespace rb4 {

struct RenderTarget;

RenderTarget*& render_target_resources_light_accumulation_target(
    RenderTargetResources& resources,
    LightAccumulationTargetKind kind);
RenderTarget* render_target_resources_light_accumulation_target(
    const RenderTargetResources& resources,
    LightAccumulationTargetKind kind);
RenderTarget* render_target_resources_create_light_accumulation_target(
    RenderTargetResources& resources,
    LightAccumulationTargetKind kind,
    RenderTarget* reusable_target);

}  // namespace rb4
