#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;

void orbis_video_output_open(OrbisRenderSystem& system);
void orbis_video_output_set_flip_rate(
    OrbisRenderSystem& system,
    std::uint32_t rate);
void orbis_video_output_set_window_margins(
    OrbisRenderSystem& system,
    std::uint32_t top,
    std::uint32_t bottom);
void orbis_create_event_queue(
    OrbisRenderSystem& system,
    const char* name);
void orbis_register_video_flip_event(OrbisRenderSystem& system);
void orbis_delete_event_queue(OrbisRenderSystem& system);
void orbis_video_output_close(OrbisRenderSystem& system);
void orbis_render_system_initialize(OrbisRenderSystem& system);
void orbis_render_system_shutdown(OrbisRenderSystem& system);
void orbis_render_system_delete(OrbisRenderSystem& system);
void orbis_create_default_vertex_buffer(OrbisRenderSystem& system);
void orbis_create_identity_instance_buffer(OrbisRenderSystem& system);
void orbis_create_back_buffer(OrbisRenderSystem& system);
void orbis_create_render_context(OrbisRenderSystem& system);
void orbis_submit_done_thread_entry(OrbisRenderSystem& system);
void orbis_submit_done_thread_run(OrbisRenderSystem& system);

}  // namespace rb4
