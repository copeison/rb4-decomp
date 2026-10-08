#pragma once

namespace rb4 {

struct RenderResourceManager;

void render_resource_manager_initialize(RenderResourceManager& manager);
void render_primary_shader_finalize(void* shader);
void render_resource_name_destruct(void* name);

}  // namespace rb4
