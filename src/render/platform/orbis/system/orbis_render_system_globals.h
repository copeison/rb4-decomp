#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisRenderContext;
struct RenderSystem;

extern OrbisRenderSystem* g_orbis_render_system;

OrbisRenderSystem* orbis_render_system_instance();
RenderSystem& orbis_render_system_base(OrbisRenderSystem& system);
OrbisRenderContext& orbis_render_system_context(OrbisRenderSystem& system);
bool orbis_frame_is_active(const OrbisRenderSystem& system);
std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system);
bool orbis_submit_token_available(const OrbisRenderSystem& system);
void orbis_consume_submit_token(OrbisRenderSystem& system);
bool orbis_submit_thread_running(const OrbisRenderSystem& system);
std::size_t orbis_active_render_frame_index();
void orbis_render_system_publish_instance(OrbisRenderSystem& system);
void orbis_render_system_clear_instance();

}  // namespace rb4
