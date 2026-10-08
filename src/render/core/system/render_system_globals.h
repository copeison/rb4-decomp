#pragma once

namespace rb4 {

struct RenderSystem;

extern RenderSystem* g_render_system;

RenderSystem* render_system_instance();
void render_system_publish_instance(RenderSystem& system);
void render_system_clear_instance();

}  // namespace rb4
