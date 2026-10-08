#include "render/platform/orbis/video/orbis_video_output.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <kernel/equeue.h>
#include <system_service.h>
#include <video_out.h>

#include "render/platform/orbis/video/orbis_back_buffer.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/core/system/render_system_frame_adapters.h"
#include "render/core/system/render_system_globals.h"
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
std::int32_t sceGnmSubmitDone();

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

bool orbis_wait_for_submit_events(
    OrbisRenderSystem& system,
    OrbisSubmitEvent* events,
    std::size_t capacity,
    std::size_t& event_count) {
    constexpr SceKernelUseconds kWaitTimeoutMicroseconds = 1'000'000;
    std::array<SceKernelEvent, 4> kernel_events{};
    auto timeout = kWaitTimeoutMicroseconds;
    int kernel_event_count = 0;
    const auto wait_capacity = static_cast<int>(
        std::min(capacity, kernel_events.size()));
    const auto result = sceKernelWaitEqueue(
        orbis_event_queue(system),
        kernel_events.data(),
        wait_capacity,
        &kernel_event_count,
        &timeout);
    if (result != 0) {
        event_count = 0;
        return false;
    }

    event_count = 0;
    for (int index = 0; index < kernel_event_count; ++index) {
        const auto filter = sceKernelGetEventFilter(&kernel_events[index]);
        if (filter == SCE_KERNEL_EVFILT_VIDEO_OUT) {
            events[event_count++].type =
                OrbisSubmitEventType::kFlipComplete;
        } else if (filter == SCE_KERNEL_EVFILT_GNM) {
            events[event_count++].type = OrbisSubmitEventType::kEndOfPipe;
        }
    }
    return true;
}

void orbis_process_submit_timeout(OrbisRenderSystem& system) {
    orbis_lock_submission(system);
    orbis_submit_scope_begin(system);
    sceGnmSubmitDone();
    orbis_submit_scope_end(system);
    orbis_unlock_submission(system);
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

void orbis_wait_for_submit_thread(OrbisRenderSystem& system) {
    orbis_lock_submission(system);
    orbis_submit_scope_begin(system);
    while (!orbis_submit_token_available(system)) {
        orbis_wait_for_submit_token(system);
    }
    orbis_submit_scope_end(system);
    orbis_unlock_submission(system);
}

// Reconstructed from eboot.elf at 0x8D77E0.
void orbis_submit_done_thread_entry(OrbisRenderSystem& system) {
    orbis_submit_done_thread_run(system);
}

// Reconstructed from eboot.elf at 0x8D7340.
void orbis_submit_done_thread_run(OrbisRenderSystem& system) {
    auto& base = orbis_render_system_base(system);
    render_system_lock(base);
    render_system_enter_locked_call(base);
    if (render_system_has_pending_frame(base)) {
        render_system_activate_pending_frame(base);
    }
    render_system_leave_locked_call(base);
    render_system_unlock(base);

    orbis_lock_submission(system);
    orbis_publish_submit_token(system);
    orbis_unlock_submission(system);
    orbis_signal_submit_condition(system);

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
