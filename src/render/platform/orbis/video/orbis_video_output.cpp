#include "render/platform/orbis/video/orbis_video_output.h"

#include <array>
#include <cstddef>
#include <kernel/equeue.h>
#include <system_service.h>
#include <video_out.h>

#include "render/platform/orbis/video/orbis_back_buffer.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/platform/orbis/system/orbis_render_system.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"
#include "render/platform/orbis/video/orbis_video_output_adapters.h"

extern "C" {

std::int32_t sceGnmAddEqEvent(
    SceKernelEqueue queue,
    std::uint32_t event_id,
    void* user_data);
std::int32_t sceGnmDeleteEqEvent(
    SceKernelEqueue queue,
    std::uint32_t event_id);

}

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

void orbis_video_output_open(OrbisRenderSystem& system) {
    constexpr std::int32_t kSystemUserId = 255;
    const auto handle = sceVideoOutOpen(kSystemUserId, 0, 0, nullptr);
    orbis_set_video_output_handle(system, handle);
}

void orbis_video_output_set_flip_rate(
    OrbisRenderSystem& system,
    std::uint32_t rate) {
    sceVideoOutSetFlipRate(
        orbis_video_output_handle(system), static_cast<std::int32_t>(rate));
}

void orbis_video_output_set_window_margins(
    OrbisRenderSystem& system,
    std::uint32_t top,
    std::uint32_t bottom) {
    sceVideoOutSetWindowModeMargins(
        orbis_video_output_handle(system),
        static_cast<int>(top),
        static_cast<int>(bottom));
}

void orbis_create_event_queue(
    OrbisRenderSystem& system,
    const char* name) {
    SceKernelEqueue queue = nullptr;
    sceKernelCreateEqueue(&queue, name);
    orbis_set_event_queue(system, queue);
}

void orbis_register_gnm_event(
    OrbisRenderSystem& system,
    std::uint32_t event_id) {
    sceGnmAddEqEvent(orbis_event_queue(system), event_id, nullptr);
}

void orbis_register_video_flip_event(OrbisRenderSystem& system) {
    sceVideoOutAddFlipEvent(
        orbis_event_queue(system),
        orbis_video_output_handle(system),
        nullptr);
}

void orbis_unregister_gnm_event(
    OrbisRenderSystem& system,
    std::uint32_t event_id) {
    sceGnmDeleteEqEvent(orbis_event_queue(system), event_id);
}

void orbis_delete_event_queue(OrbisRenderSystem& system) {
    sceKernelDeleteEqueue(orbis_event_queue(system));
}

void orbis_video_output_close(OrbisRenderSystem& system) {
    sceVideoOutClose(orbis_video_output_handle(system));
}

void orbis_hide_system_splash_screen() {
    sceSystemServiceHideSplashScreen();
}

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
    orbis_set_submit_thread_running(system, true);
    orbis_consume_submit_token(system);
    orbis_initialize_submit_profiler(system);
    orbis_wait_for_submit_thread(system);
    orbis_hide_system_splash_screen();
}

// Reconstructed from eboot.elf at 0x8D8040.
void orbis_render_system_shutdown(OrbisRenderSystem& system) {
    orbis_set_submit_thread_running(system, false);
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
