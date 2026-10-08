#include "render/core/system/render_system_frame.h"

#include <algorithm>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/time/performance_counter.h"
#include "render/core/context/render_context.h"
#include "render/core/context/render_context_adapters.h"
#include "render/core/frame/render_frame_owner.h"
#include "render/core/synchronization/render_system_lock.h"
#include "render/core/system/render_epoch.h"
#include "render/core/system/render_system.h"
#include "render/core/system/render_system_frame_adapters.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_runtime_adapters.h"
#include "render/core/system/render_system_state.h"

namespace rb4 {

namespace {

// Reconstructed from eboot.elf at 0x3DF000.
void append_default_target_states(
    RenderTargetStateArray& states,
    std::size_t count) {
    const auto available = states.end == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(states.capacity - states.end);
    if (available >= count) {
        if (count != 0) {
            std::memset(states.end, 0, count * sizeof(*states.end));
            states.end += count;
        }
        return;
    }

    const auto current_size = states.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(states.end - states.begin);
    const auto doubled_capacity = current_size == 0
        ? std::size_t{1}
        : current_size * 2;
    const auto new_capacity =
        std::max(doubled_capacity, current_size + count);
    auto** new_begin = static_cast<RenderTargetState**>(
        engine_allocate_sized(
            new_capacity * sizeof(RenderTargetState*)));

    if (current_size != 0) {
        std::memmove(
            new_begin,
            states.begin,
            current_size * sizeof(*new_begin));
    }
    std::memset(
        new_begin + current_size,
        0,
        count * sizeof(*new_begin));

    if (states.begin != nullptr) {
        engine_deallocate_sized(
            states.begin,
            static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(states.capacity) -
                reinterpret_cast<std::uint8_t*>(states.begin)));
    }

    states.begin = new_begin;
    states.end = new_begin + current_size + count;
    states.capacity = new_begin + new_capacity;
}

void resize_target_states(
    RenderTargetStateArray& states,
    std::size_t count) {
    const auto current_size = states.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(states.end - states.begin);
    if (count > current_size) {
        append_default_target_states(states, count - current_size);
        return;
    }

    states.end = count == 0 ? states.begin : states.begin + count;
}

// Reconstructed from eboot.elf at 0x3DE3A0.
bool render_system_attach_frame_owner(
    RenderSystem& system,
    RenderFrameOwner& owner) {
    auto& runtime = render_system_core_state(system);
    static_cast<void>(scePthreadSelf());
    runtime.active_frame_owner = &owner;
    render_frame_owner_begin(owner);

    if (render_frame_owner_output_extent(owner).empty()) {
        static_cast<void>(scePthreadSelf());
        runtime.active_target_states.end =
            runtime.active_target_states.begin;
        runtime.active_frame_owner = nullptr;
        return false;
    }

    runtime.submitted_frame_owners.items[
        runtime.submitted_frame_owners.count++] = &owner;

    const auto targets = render_frame_owner_target_states(owner);
    resize_target_states(runtime.active_target_states, targets.count);
    std::copy_n(
        targets.states,
        targets.count,
        runtime.active_target_states.begin);

    render_context_begin_frame(*runtime.render_context, 0);
    return true;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DE170.
void render_system_prepare_frame(
    RenderSystem& system,
    bool auxiliary_frame) {
    render_system_acquire_frame_lock(system);

    auto& runtime = render_system_core_state(system);
    runtime.frame_in_progress = true;
    render_system_platform_prepare_frame(system, auxiliary_frame);

    if (!auxiliary_frame) {
        if (render_system_frame_phase(system) == 0) {
            render_system_update_frame_phase_metrics(system);
        }

        const auto current_counter = performance_counter_read();
        const auto elapsed_ticks = runtime.frame_timing_initialized != 0
            ? current_counter - runtime.previous_frame_counter
            : runtime.initial_frame_tick_span;
        runtime.previous_frame_counter = current_counter;
        runtime.initial_frame_tick_span = 0;
        runtime.frame_timing_initialized = 1;

        const auto elapsed_milliseconds =
            performance_counter_ticks_to_milliseconds(elapsed_ticks);
        runtime.instantaneous_frame_rate = static_cast<float>(
            1000.0 / elapsed_milliseconds);
        if (runtime.smoothed_frame_rate == 0.0F) {
            runtime.smoothed_frame_rate =
                runtime.instantaneous_frame_rate;
        }
        runtime.smoothed_frame_rate =
            (runtime.smoothed_frame_rate * 59.0F +
             runtime.instantaneous_frame_rate) /
            60.0F;
    }

    runtime.render_context->frame_active = true;
    if (runtime.frame_activation_pending) {
        render_system_activate_pending_frame(system);
    }
    runtime.gpu_frame_stat = render_system_begin_gpu_frame_tracking(
        system, *runtime.render_context);
}

// Reconstructed from eboot.elf at 0x3DE4A0.
void render_system_finish_frame(
    RenderSystem& system,
    bool auxiliary_frame) {
    auto& runtime = render_system_core_state(system);
    if (runtime.frame_activation_pending) {
        render_system_activate_pending_frame(system);
    }

    render_system_end_gpu_frame_tracking(
        system,
        *runtime.render_context,
        runtime.gpu_frame_stat);

    if (auxiliary_frame) {
        render_system_platform_submit_frame(
            system, runtime.submitted_frame_owners, true);
        ++runtime.auxiliary_frame_epoch;
    } else {
        render_system_finalize_primary_context(
            system, *runtime.render_context);
        render_system_platform_submit_frame(
            system, runtime.submitted_frame_owners, false);
        if (render_system_frame_phase(system) == 1) {
            render_system_update_frame_phase_metrics(system);
        }
        runtime.submitted_frame_owners.count = 0;
        ++runtime.frame_epoch;
        render_system_flush_deferred_releases(system);
    }

    runtime.frame_in_progress = false;
    runtime.render_context->frame_active = false;
    render_system_release_frame_lock(system);

    if (!auxiliary_frame) {
        render_system_poll_default_resources(system);
    }
}

// Reconstructed from eboot.elf at 0x3DE8F0.
void render_system_begin_auxiliary_frame(
    RenderSystem& system,
    RenderTargetState* target_state) {
    render_system_prepare_frame(system, true);

    auto& runtime = render_system_core_state(system);
    const auto target_count = target_state == nullptr
        ? std::size_t{0}
        : std::size_t{1};
    resize_target_states(runtime.active_target_states, target_count);
    if (target_state != nullptr) {
        runtime.active_target_states.begin[0] = target_state;
    }
    render_context_begin_frame(*runtime.render_context, 0);
}

// Reconstructed from eboot.elf at 0x3DE9E0.
void render_system_finish_auxiliary_frame(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    runtime.active_target_states.end = runtime.active_target_states.begin;
    render_system_finish_frame(system, true);
}

// Reconstructed from eboot.elf at 0x3DE0E0.
void render_system_poll() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    render_system_lock(*system);
    render_system_enter_locked_call(*system);

    auto* frame_owner = render_system_frame_owner(*system);
    if (frame_owner != nullptr) {
        render_frame_owner_poll(*frame_owner);
    }

    render_system_leave_locked_call(*system);
    render_system_unlock(*system);
}

bool render_system_is_alive() {
    return render_system_instance() != nullptr;
}

// Reconstructed from eboot.elf at 0x3DE130.
bool render_system_begin_frame() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return false;
    }

    render_system_prepare_frame(*system, false);
    auto* frame_owner = render_system_frame_owner(*system);
    if (frame_owner != nullptr &&
        render_system_attach_frame_owner(*system, *frame_owner)) {
        return true;
    }

    render_system_finish_frame(*system, false);
    return false;
}

// Reconstructed from eboot.elf at 0x3DE7C0.
void render_system_end_frame() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    auto& runtime = render_system_core_state(*system);
    static_cast<void>(scePthreadSelf());
    runtime.active_target_states.end = runtime.active_target_states.begin;
    runtime.active_frame_owner = nullptr;
    render_system_finish_frame(*system, false);
}

// Reconstructed from eboot.elf at 0x3DEAA0.
void render_system_skip_frame() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    render_system_lock(*system);
    advance_render_epoch(*system);
    render_system_unlock(*system);
}

}  // namespace rb4
