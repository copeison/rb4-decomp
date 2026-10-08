#pragma once

namespace rb4 {

struct RenderLightingResources;

void render_lighting_resources_initialize(RenderLightingResources& resources);
void render_lighting_owned_state_release(void* state);

}  // namespace rb4
