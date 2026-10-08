#pragma once

namespace rb4 {

struct OrbisRenderSystem;

OrbisRenderSystem* orbis_render_system_create();
void orbis_render_system_construct(OrbisRenderSystem& system);
void orbis_render_system_destruct(OrbisRenderSystem& system);

}  // namespace rb4
