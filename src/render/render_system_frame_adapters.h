#pragma once

#include <cstdint>

namespace rb4 {

struct RenderFrameOwner;
struct RenderTarget;
struct RenderSystem;

RenderSystem* render_system_instance();

void render_system_lock(RenderSystem& system);
void render_system_unlock(RenderSystem& system);
void render_system_enter_locked_call(RenderSystem& system);
void render_system_leave_locked_call(RenderSystem& system);

RenderFrameOwner* render_system_frame_owner(RenderSystem& system);
RenderFrameOwner* render_system_active_frame_owner(RenderSystem& system);
void render_frame_owner_poll(RenderFrameOwner& owner);
const RenderTarget* render_frame_owner_primary_target(
    const RenderFrameOwner& owner);
std::uint32_t render_target_width(const RenderTarget& target);
std::uint32_t render_target_height(const RenderTarget& target);
std::uint32_t render_target_draw_mode(const RenderTarget& target);
std::uint32_t render_target_debug_view(const RenderTarget& target);

void render_system_prepare_frame(RenderSystem& system, bool auxiliary_frame);
bool render_system_attach_frame_owner(
    RenderSystem& system,
    RenderFrameOwner& owner);
void render_system_clear_active_frame(RenderSystem& system);
void render_system_finish_frame(RenderSystem& system, bool auxiliary_frame);
void render_system_increment_skipped_frame_count(RenderSystem& system);

}  // namespace rb4
