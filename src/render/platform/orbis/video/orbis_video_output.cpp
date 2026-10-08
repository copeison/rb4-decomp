#include "render/platform/orbis/video/orbis_video_output.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <kernel/equeue.h>
#include <system_service.h>
#include <video_out.h>

#include "core/memory/engine_memory.h"
#include "core/threading/engine_thread.h"
#include "core/time/performance_counter.h"
#include "render/core/settings/render_settings.h"
#include "render/core/synchronization/render_system_lock.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/platform/orbis/meshes/orbis_builtin_buffers.h"
#include "render/platform/orbis/system/orbis_render_system.h"
#include "render/platform/orbis/system/orbis_render_factory.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"
#include "render/platform/orbis/textures/orbis_texture_2d.h"
#include "render/platform/orbis/video/orbis_back_buffer.h"

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
constexpr std::uint32_t kGnmEventId = 64;
constexpr std::uint32_t kSubmitThreadPriority = 699;
constexpr float kSubmitDoneTimeoutMilliseconds = 1000.0F;
constexpr std::size_t kBackBufferCount = 2;
constexpr std::int64_t kInitialPreviousBuffer = 2;

struct OrbisSubmitWorkerState {
    std::size_t next_buffer = 0;
    std::int64_t previous_buffer = kInitialPreviousBuffer;
    std::uint64_t last_submit_check = 0;
    std::uint64_t pending_submit_ticks = 0;
};

template <typename Callback>
void for_each_output_texture(
    OrbisRenderSystem& system,
    Callback callback) {
    auto& base = orbis_render_system_base(system);
    auto* frame_owner = render_system_frame_owner(base);
    const auto states = render_frame_owner_target_states(*frame_owner);
    for (std::size_t index = 0; index < states.count; ++index) {
        auto* texture = render_target_state_texture(*states.states[index]);
        if (texture != nullptr) {
            callback(reinterpret_cast<OrbisTexture2D&>(*texture));
        }
    }
}

std::uint32_t video_flip_mode(std::int32_t rate) {
    return rate == 0
        ? SCE_VIDEO_OUT_FLIP_MODE_HSYNC
        : SCE_VIDEO_OUT_FLIP_MODE_WINDOW_2;
}

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

void orbis_process_submit_timeout(
    OrbisRenderSystem& system,
    OrbisSubmitWorkerState& state) {
    orbis_lock_submission(system);
    orbis_submit_scope_begin(system);
    state.last_submit_check = performance_counter_read();
    sceGnmSubmitDone();
    state.pending_submit_ticks = 0;
    orbis_submit_scope_end(system);
    orbis_unlock_submission(system);
}

void orbis_process_flip_complete(OrbisRenderSystem& system) {
    SceVideoOutFlipStatus status{};
    sceVideoOutGetFlipStatus(orbis_video_output_handle(system), &status);

    const auto completed_buffer =
        static_cast<std::uint64_t>(status.flipArg);
    if (completed_buffer >= kBackBufferCount) {
        return;
    }

    for_each_output_texture(
        system,
        [completed_buffer](OrbisTexture2D& texture) {
            orbis_texture_2d_complete_pending_presentation(
                texture, static_cast<std::size_t>(completed_buffer));
        });
}

void orbis_process_end_of_pipe(
    OrbisRenderSystem& system,
    OrbisSubmitWorkerState& state) {
    orbis_lock_submission(system);
    orbis_submit_scope_begin(system);

    auto& context = orbis_render_system_context(system);
    bool submit_done = orbis_render_context_frame_submissions_complete(
        context, state.next_buffer);
    if (!submit_done) {
        const auto now = performance_counter_read();
        state.pending_submit_ticks += now - state.last_submit_check;
        state.last_submit_check = now;
        submit_done = static_cast<float>(
            performance_counter_ticks_to_milliseconds(
                state.pending_submit_ticks)) >=
            kSubmitDoneTimeoutMilliseconds;
    }

    if (submit_done) {
        state.last_submit_check = performance_counter_read();
        sceGnmSubmitDone();
        state.pending_submit_ticks = 0;
    }

    for_each_output_texture(
        system,
        [&state](OrbisTexture2D& texture) {
            orbis_texture_2d_add_pending_presentation(
                texture, state.next_buffer);
        });

    orbis_publish_submit_token(system);
    orbis_submit_scope_end(system);
    orbis_unlock_submission(system);
    orbis_signal_submit_condition(system);

    auto& base = orbis_render_system_base(system);
    const auto rate = render_settings_active_vsync_mode(
        *render_system_settings(base));
    if (rate != orbis_cached_flip_rate(system)) {
        orbis_set_cached_flip_rate(system, rate);
        orbis_video_output_set_flip_rate(system, rate == 2);
    }

    sceVideoOutSubmitFlip(
        orbis_video_output_handle(system),
        static_cast<std::int32_t>(state.next_buffer),
        video_flip_mode(rate),
        state.previous_buffer);
    state.previous_buffer = static_cast<std::int64_t>(state.next_buffer);
    state.next_buffer = (state.next_buffer + 1) % kBackBufferCount;
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
    auto& base = orbis_render_system_base(system);
    render_system_set_factory(base, orbis_render_factory_create());
    orbis_create_back_buffer(system);
    orbis_create_render_context(system);

    orbis_initialize_submit_condition(system);
    engine_thread_configure(
        orbis_submit_thread_wrapper(system),
        orbis_submit_done_thread_entry,
        &system,
        kSubmitThreadName,
        -1,
        kSubmitThreadPriority,
        0,
        0);
    orbis_set_submit_thread_running(system, true);
    orbis_consume_submit_token(system);
    engine_thread_start(orbis_submit_thread(system));
    orbis_wait_for_submit_thread(system);
    orbis_hide_system_splash_screen();
}

// Reconstructed from eboot.elf at 0x8D8040.
void orbis_render_system_shutdown(OrbisRenderSystem& system) {
    orbis_set_submit_thread_running(system, false);
    engine_thread_join(orbis_submit_thread(system));
    orbis_destroy_submit_condition(system);
    auto& base = orbis_render_system_base(system);
    render_system_release_back_buffer(base);
    render_system_release_render_contexts(base);
    orbis_unregister_gnm_event(system, kGnmEventId);
    orbis_delete_event_queue(system);
    orbis_video_output_close(system);
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
std::int32_t orbis_submit_done_thread_entry(void* context) {
    auto& system = *static_cast<OrbisRenderSystem*>(context);
    orbis_submit_done_thread_run(system);
    return 0;
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

    OrbisSubmitWorkerState state;
    state.last_submit_check = performance_counter_read();
    std::array<OrbisSubmitEvent, 4> events{};
    while (orbis_submit_thread_running(system)) {
        std::size_t event_count = 0;
        if (!orbis_wait_for_submit_events(
                system, events.data(), events.size(), event_count)) {
            orbis_process_submit_timeout(system, state);
            continue;
        }

        for (std::size_t index = 0; index < event_count; ++index) {
            switch (events[index].type) {
            case OrbisSubmitEventType::kFlipComplete:
                orbis_process_flip_complete(system);
                break;
            case OrbisSubmitEventType::kEndOfPipe:
                orbis_process_end_of_pipe(system, state);
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
