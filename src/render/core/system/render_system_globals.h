#pragma once

#include <cstddef>

namespace rb4 {

struct RenderFrameOwner;
struct RenderSettings;
struct RenderSystem;

extern RenderSystem* g_render_system;

RenderSystem* render_system_instance();
RenderFrameOwner* render_system_frame_owner(RenderSystem& system);
RenderFrameOwner& render_system_primary_frame_owner(RenderSystem& system);
std::size_t render_system_frame_owner_count(const RenderSystem& system);
RenderFrameOwner& render_system_frame_owner_at(
    RenderSystem& system,
    std::size_t index);
bool render_system_has_pending_frame(const RenderSystem& system);
void render_system_activate_pending_frame(RenderSystem& system);
RenderSettings* render_system_settings(RenderSystem& system);
void render_system_set_settings(
    RenderSystem& system,
    RenderSettings* settings);
void render_system_publish_instance(RenderSystem& system);
void render_system_clear_instance();

}  // namespace rb4
