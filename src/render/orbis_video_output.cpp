#include "orbis_video_output.h"

#include <array>
#include <cstddef>

#include "orbis_back_buffer.h"
#include "orbis_render_context.h"
#include "orbis_render_system.h"
#include "orbis_video_output_adapters.h"

namespace rb4 {

namespace {

constexpr const char* kEventQueueName = "EOP QUEUE";
constexpr const char* kSubmitThreadName = "SubmitDoneThread";
constexpr const char* kDefaultVertexBufferName = "DefaultVBuffer";
constexpr const char* kIdentityInstanceBufferName =
    "IdentityInstanceVBuffer";
constexpr std::uint32_t kGnmEventId = 64;
constexpr std::uint32_t kSubmitThreadPriority = 699;

}  // namespace

// Reconstructed from eboot.elf at 0x8D7B20.
void orbis_render_system_initialize(OrbisRenderSystem& system) {
    orbis_video_output_open(system);
    orbis_video_output_set_flip_rate(system, 0);
    orbis_video_output_set_window_margins(system, 1080, 0);

    orbis_create_event_queue(system, kEventQueueName);
    orbis_register_gnm_event(system, kGnmEventId);
    orbis_register_video_flip_event(system);

    orbis_create_default_vertex_buffer(system);
    orbis_create_identity_instance_buffer(system);
    orbis_register_render_factories(system);
    orbis_create_back_buffer(system);
    orbis_create_render_context(system);

    orbis_initialize_submit_condition(system);
    orbis_start_submit_thread(
        system,
        orbis_submit_done_thread_entry,
        kSubmitThreadName,
        kSubmitThreadPriority);
    orbis_initialize_submit_profiler(system);
    orbis_wait_for_submit_thread(system);
    orbis_hide_system_splash_screen();
}

// Reconstructed from eboot.elf at 0x8D8040.
void orbis_render_system_shutdown(OrbisRenderSystem& system) {
    orbis_request_submit_thread_stop(system);
    orbis_join_submit_thread(system);
    orbis_destroy_submit_condition(system);
    orbis_release_frame_runtime(system);
    orbis_unregister_gnm_event(system, kGnmEventId);
    orbis_delete_event_queue(system);
    orbis_video_output_close(system);
}

// Reconstructed from eboot.elf at 0x8D7DB0.
void orbis_create_default_vertex_buffer(OrbisRenderSystem& system) {
    auto& buffer = orbis_allocate_default_vertex_buffer(
        system, kDefaultVertexBufferName);
    orbis_upload_default_vertex_data(system, buffer);
    orbis_bind_default_vertex_buffer(system, buffer);
}

// Reconstructed from eboot.elf at 0x8D7EB0.
void orbis_create_identity_instance_buffer(OrbisRenderSystem& system) {
    auto& buffer = orbis_allocate_identity_instance_buffer(
        system, kIdentityInstanceBufferName, 120);
    orbis_upload_identity_instance_data(system, buffer);
    orbis_bind_identity_instance_buffer(system, buffer);
}

void orbis_create_back_buffer(OrbisRenderSystem& system) {
    static_cast<void>(orbis_back_buffer_create(system));
}

void orbis_create_render_context(OrbisRenderSystem& system) {
    static_cast<void>(orbis_render_context_create(system));
}

// Reconstructed from eboot.elf at 0x8D77E0.
void orbis_submit_done_thread_entry(OrbisRenderSystem& system) {
    orbis_submit_done_thread_run(system);
}

// Reconstructed from eboot.elf at 0x8D7340.
void orbis_submit_done_thread_run(OrbisRenderSystem& system) {
    std::array<OrbisSubmitEvent, 4> events{};
    while (orbis_submit_thread_running(system)) {
        std::size_t event_count = 0;
        if (!orbis_wait_for_submit_events(
                system, events.data(), events.size(), event_count)) {
            orbis_process_submit_timeout(system);
            continue;
        }

        for (std::size_t index = 0; index < event_count; ++index) {
            switch (events[index].type) {
            case OrbisSubmitEventType::kFlipComplete:
                orbis_process_flip_complete(system, events[index]);
                break;
            case OrbisSubmitEventType::kEndOfPipe:
                orbis_process_end_of_pipe(system, events[index]);
                break;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x8D7B00.
void orbis_render_system_delete(OrbisRenderSystem& system) {
    orbis_render_system_destruct(system);
    render_free(&system);
}

}  // namespace rb4
