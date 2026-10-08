#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;

extern OrbisRenderSystem* g_orbis_render_system;

OrbisRenderSystem* orbis_render_system_instance();
std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system);
void orbis_render_system_publish_instance(OrbisRenderSystem& system);
void orbis_render_system_clear_instance();

}  // namespace rb4
