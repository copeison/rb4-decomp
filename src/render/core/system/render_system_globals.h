#pragma once

namespace rb4 {

struct RenderFrameOwner;
struct RenderSystem;

extern RenderSystem* g_render_system;

RenderSystem* render_system_instance();
RenderFrameOwner* render_system_frame_owner(RenderSystem& system);
void render_system_publish_instance(RenderSystem& system);
void render_system_clear_instance();

}  // namespace rb4
