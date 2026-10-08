#pragma once

namespace rb4 {

struct OrbisRenderSystem;

void orbis_render_system_initialize(OrbisRenderSystem& system);
void orbis_render_system_delete(OrbisRenderSystem& system);
void orbis_create_default_vertex_buffer(OrbisRenderSystem& system);
void orbis_create_identity_instance_buffer(OrbisRenderSystem& system);
void orbis_submit_done_thread_entry(OrbisRenderSystem& system);
void orbis_submit_done_thread_run(OrbisRenderSystem& system);

}  // namespace rb4
