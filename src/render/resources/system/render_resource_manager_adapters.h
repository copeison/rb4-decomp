#pragma once

namespace rb4 {

struct RenderResourceManager;

void render_resource_manager_initialize(RenderResourceManager& manager);
void render_resource_manager_finalize(RenderResourceManager& manager);
void render_resource_manager_shutdown(RenderResourceManager& manager);

}  // namespace rb4
