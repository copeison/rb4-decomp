#pragma once

namespace rb4 {

struct RenderResourceManager;

void render_resource_manager_initialize(RenderResourceManager& manager);
void render_resource_manager_finalize(RenderResourceManager& manager);
void render_resource_name_destruct(void* name);
void render_resource_specialized_state_destruct(void* state);

}  // namespace rb4
