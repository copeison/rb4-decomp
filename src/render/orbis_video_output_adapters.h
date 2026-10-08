#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisVertexBuffer;

enum class OrbisSubmitEventType {
    kFlipComplete,
    kEndOfPipe,
};

struct OrbisSubmitEvent {
    OrbisSubmitEventType type;
};

using OrbisSubmitThreadEntry = void (*)(OrbisRenderSystem& system);

void orbis_video_output_open(OrbisRenderSystem& system);
void orbis_video_output_set_flip_rate(
    OrbisRenderSystem& system,
    std::uint32_t rate);
void orbis_video_output_set_window_margins(
    OrbisRenderSystem& system,
    std::uint32_t vertical,
    std::uint32_t horizontal);
void orbis_create_event_queue(
    OrbisRenderSystem& system,
    const char* name);
void orbis_register_gnm_event(
    OrbisRenderSystem& system,
    std::uint32_t event_id);
void orbis_register_video_flip_event(OrbisRenderSystem& system);
void orbis_register_render_factories(OrbisRenderSystem& system);

void orbis_initialize_submit_condition(OrbisRenderSystem& system);
void orbis_start_submit_thread(
    OrbisRenderSystem& system,
    OrbisSubmitThreadEntry entry,
    const char* name,
    std::uint32_t priority);
void orbis_initialize_submit_profiler(OrbisRenderSystem& system);
void orbis_wait_for_submit_thread(OrbisRenderSystem& system);
void orbis_hide_system_splash_screen();

OrbisVertexBuffer& orbis_allocate_default_vertex_buffer(
    OrbisRenderSystem& system,
    const char* name);
void orbis_upload_default_vertex_data(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);
void orbis_bind_default_vertex_buffer(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);

OrbisVertexBuffer& orbis_allocate_identity_instance_buffer(
    OrbisRenderSystem& system,
    const char* name,
    std::size_t size);
void orbis_upload_identity_instance_data(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);
void orbis_bind_identity_instance_buffer(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);

bool orbis_submit_thread_running(const OrbisRenderSystem& system);
bool orbis_wait_for_submit_events(
    OrbisRenderSystem& system,
    OrbisSubmitEvent* events,
    std::size_t capacity,
    std::size_t& event_count);
void orbis_process_submit_timeout(OrbisRenderSystem& system);
void orbis_process_flip_complete(
    OrbisRenderSystem& system,
    const OrbisSubmitEvent& event);
void orbis_process_end_of_pipe(
    OrbisRenderSystem& system,
    const OrbisSubmitEvent& event);

void render_free(void* allocation);

}  // namespace rb4
