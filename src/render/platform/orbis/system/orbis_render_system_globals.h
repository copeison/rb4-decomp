#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisRenderContext;

extern OrbisRenderSystem* g_orbis_render_system;

OrbisRenderSystem* orbis_render_system_instance();
OrbisRenderContext& orbis_render_system_context(OrbisRenderSystem& system);
bool orbis_frame_is_active(const OrbisRenderSystem& system);
std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system);
void orbis_render_system_publish_instance(OrbisRenderSystem& system);
void orbis_render_system_clear_instance();

}  // namespace rb4
