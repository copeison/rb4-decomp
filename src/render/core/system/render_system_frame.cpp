#include "render/core/system/render_system_frame.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "utl/time/Timer.h"
#include "render/core/context/render_context.h"
#include "render/core/context/render_context_adapters.h"
#include "render/core/debug/render_gpu_stat_block.h"
#include "render/core/frame/render_frame_owner.h"
#include "render/core/settings/render_settings.h"
#include "render/core/synchronization/render_deferred_release.h"
#include "render/core/synchronization/render_system_lock.h"
#include "render/core/system/render_epoch.h"
#include "render/core/system/render_system.h"
#include "render/core/system/render_system_frame_adapters.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/core/targets/render_target_resources.h"
#include "render/resources/audio/audio_analysis_textures.h"
#include "render/resources/system/default_render_resources.h"
#include "render/resources/video/bink_render_manager.h"
#include "render/resources/video/bink_render_manager_adapters.h"

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
        HmxAllocator::gStlAllocator.allocate(
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
        HmxAllocator::gStlAllocator.deallocate(
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

// Reconstructed from the primary-frame branch of eboot.elf at 0x3DE4A0.
void prepare_primary_context_submission(
    RenderSystem& system,
    RenderContext& context) {
    constexpr std::size_t kMaxSubmissionResources = 12;
    std::array<
        RenderContextSubmissionResource,
        kMaxSubmissionResources> resources{};
    std::size_t resource_count = 0;

    const auto& owners =
        render_system_core_state(system).submitted_frame_owners;
    for (std::size_t owner_index = 0;
         owner_index < owners.count;
         ++owner_index) {
        const auto& owner = *owners.items[owner_index];
        if (render_frame_owner_output_extent(owner).empty()) {
            continue;
        }

        const auto targets = render_frame_owner_target_states(owner);
        for (std::size_t target_index = 0;
             target_index < targets.count;
             ++target_index) {
            const auto& target_resources =
                reinterpret_cast<const RenderTargetResources&>(
                    *targets.states[target_index]);
            resources[resource_count++] = {
                nullptr,
                target_resources.source_texture,
                -1,
                4,
            };
        }
    }

    render_context_prepare_submission_resources(
        context, resources.data(), resource_count);
}

void update_frame_phase_state(RenderSystem& system) {
    render_frame_phase_callbacks(false);
    const auto& runtime = render_system_core_state(system);
    if (runtime.settings->partial_framerate_enabled) {
        render_set_partial_frame_phase(runtime.frame_epoch & 1U);
    }
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
            update_frame_phase_state(system);
        }

        const auto current_counter = Hmx::Timer::GetCycleCounter();
        const auto elapsed_ticks = runtime.frame_timing_initialized != 0
            ? current_counter - runtime.previous_frame_counter
            : runtime.initial_frame_tick_span;
        runtime.previous_frame_counter = current_counter;
        runtime.initial_frame_tick_span = 0;
        runtime.frame_timing_initialized = 1;

        const auto elapsed_milliseconds =
            Hmx::Timer::CyclesToMs(elapsed_ticks);
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
    runtime.gpu_frame_stat_id = render_gpu_stat_block_begin(
        render_system_gpu_stat_block(system),
        *runtime.render_context,
        "GPU Total");
    audio_analysis_textures_prepare_frame(
        render_system_audio_analysis_textures(system),
        *runtime.render_context);
    bink_render_manager_prepare_frame(
        bink_render_manager_instance(),
        *runtime.render_context);
}

// Reconstructed from eboot.elf at 0x3DE4A0.
void render_system_finish_frame(
    RenderSystem& system,
    bool auxiliary_frame) {
    auto& runtime = render_system_core_state(system);
    if (runtime.frame_activation_pending) {
        render_system_activate_pending_frame(system);
    }

    render_gpu_stat_block_end(
        render_system_gpu_stat_block(system),
        *runtime.render_context,
        runtime.gpu_frame_stat_id);

    if (auxiliary_frame) {
        render_system_platform_submit_frame(
            system, runtime.submitted_frame_owners, true);
        ++runtime.auxiliary_frame_epoch;
    } else {
        render_gpu_stat_block_finish_frame(
            render_system_gpu_stat_block(system));
        prepare_primary_context_submission(
            system, *runtime.render_context);
        render_system_platform_submit_frame(
            system, runtime.submitted_frame_owners, false);
        if (render_system_frame_phase(system) == 1) {
            update_frame_phase_state(system);
        }
        runtime.submitted_frame_owners.count = 0;
        ++runtime.frame_epoch;
        render_system_flush_deferred_releases(system);
    }

    runtime.frame_in_progress = false;
    runtime.render_context->frame_active = false;
    render_system_release_frame_lock(system);

    if (!auxiliary_frame) {
        render_poll_default_resources(
            render_system_default_resources(system));
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
