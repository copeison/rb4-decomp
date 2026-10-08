#include "render/core/system/render_system_frame.h"

#include <algorithm>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "render/core/context/render_context_adapters.h"
#include "render/core/frame/render_frame_owner.h"
#include "render/core/synchronization/render_system_lock.h"
#include "render/core/system/render_epoch.h"
#include "render/core/system/render_system_frame_adapters.h"
#include "render/core/system/render_system_globals.h"
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
