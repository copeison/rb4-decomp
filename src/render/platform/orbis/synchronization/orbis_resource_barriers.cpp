#include "render/platform/orbis/synchronization/orbis_resource_barriers.h"

#include <cstdint>

#include "render/platform/orbis/synchronization/orbis_resource_barriers_adapters.h"
#include "render/platform/orbis/synchronization/orbis_resource_sync.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kInvalidateL1 = 0x10;
constexpr std::uint32_t kWriteBackAndInvalidateL1L2 = 0x38;

bool is_state(
    RenderResourceState state,
    RenderResourceState expected) {
    return state == expected;
}

bool is_write_destination(RenderResourceState state) {
    return is_state(state, RenderResourceState::kRenderTarget) ||
        is_state(state, RenderResourceState::kUnorderedAccess) ||
        is_state(state, RenderResourceState::kDepthWrite) ||
        is_state(state, RenderResourceState::kStreamOutput) ||
        is_state(state, RenderResourceState::kCopyDestination) ||
        is_state(state, RenderResourceState::kResolveDestination);
}

std::uint32_t destination_cache_actions(RenderResourceState state) {
    if (is_state(state, RenderResourceState::kUnorderedAccess) ||
        is_state(state, RenderResourceState::kStreamOutput) ||
        is_state(state, RenderResourceState::kCopyDestination)) {
        return kWriteBackAndInvalidateL1L2;
    }
    return 0;
}

void synchronize_barrier_phase(
    OrbisRenderContext& context,
    const RenderResourceBarrier& barrier,
    std::uint32_t barrier_cache_actions,
    volatile std::uint32_t*& shared_label,
    std::uint32_t& cache_actions,
    bool& needs_completion_wait) {
    switch (barrier.phase) {
    case RenderResourceBarrierPhase::kImmediate:
        cache_actions |= barrier_cache_actions;
        needs_completion_wait = true;
        break;
    case RenderResourceBarrierPhase::kBegin:
        orbis_render_context_signal_resource(
            context, barrier.resource, shared_label);
        cache_actions |= barrier_cache_actions;
        break;
    case RenderResourceBarrierPhase::kEnd:
        orbis_render_context_wait_for_resource(
            context, barrier.resource);
        break;
    }
}

void resolve_texture_metadata(
    OrbisRenderContext& context,
    const RenderResourceBarrier& barrier,
    bool resolve_depth,
    bool& needs_completion_wait) {
    if (resolve_depth) {
        orbis_render_context_resolve_depth_metadata(
            context, barrier.resource, barrier.subresource);
    } else {
        orbis_render_context_resolve_color_metadata(
            context, barrier.resource, barrier.subresource);
    }

    volatile std::uint32_t* metadata_label = nullptr;
    if (orbis_render_context_recording_compute(context)) {
        const auto compute_queue =
            orbis_render_context_active_compute_queue(context);
        orbis_render_context_select_graphics(context);
        orbis_render_context_signal_resource(
            context, barrier.resource, metadata_label);
        orbis_render_context_select_compute(context, compute_queue);

        if (barrier.phase != RenderResourceBarrierPhase::kBegin) {
            orbis_render_context_wait_for_resource(
                context, barrier.resource);
        }
    } else if (barrier.phase == RenderResourceBarrierPhase::kBegin) {
        orbis_render_context_signal_resource(
            context, barrier.resource, metadata_label);
    } else {
        needs_completion_wait = true;
    }
}

void process_transition(
    OrbisRenderContext& context,
    const RenderResourceBarrier& barrier,
    volatile std::uint32_t*& shared_label,
    std::uint32_t& cache_actions,
    bool& needs_completion_wait) {
    if (barrier.state_before == barrier.state_after) {
        return;
    }

    if (is_state(
            barrier.state_after,
            RenderResourceState::kRenderTarget)) {
        orbis_render_context_wait_for_render_target(
            context, barrier.resource);
    }

    const auto barrier_cache_actions =
        destination_cache_actions(barrier.state_after);

    if (is_state(
            barrier.state_before,
            RenderResourceState::kRenderTarget)) {
        if (barrier.phase == RenderResourceBarrierPhase::kEnd) {
            orbis_render_context_wait_for_resource(
                context, barrier.resource);
        } else {
            resolve_texture_metadata(
                context, barrier, false, needs_completion_wait);
        }
        return;
    }

    if (is_state(
            barrier.state_before,
            RenderResourceState::kDepthWrite)) {
        if (barrier.phase == RenderResourceBarrierPhase::kEnd) {
            orbis_render_context_wait_for_resource(
                context, barrier.resource);
        } else {
            resolve_texture_metadata(
                context, barrier, true, needs_completion_wait);
        }
        return;
    }

    if (is_state(
            barrier.state_before,
            RenderResourceState::kResolveDestination)) {
        return;
    }

    const bool source_requires_barrier =
        is_state(
            barrier.state_before,
            RenderResourceState::kUnorderedAccess) ||
        is_state(
            barrier.state_before,
            RenderResourceState::kStreamOutput) ||
        is_state(
            barrier.state_before,
            RenderResourceState::kCopyDestination);
    if (source_requires_barrier || is_write_destination(barrier.state_after)) {
        synchronize_barrier_phase(
            context,
            barrier,
            barrier_cache_actions,
            shared_label,
            cache_actions,
            needs_completion_wait);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x8EAA10.
void orbis_render_context_resource_barriers(
    OrbisRenderContext& context,
    std::size_t barrier_count,
    const RenderResourceBarrier* barriers) {
    volatile std::uint32_t* shared_label = nullptr;
    std::uint32_t cache_actions = 0;
    bool needs_completion_wait = false;

    for (std::size_t index = 0; index < barrier_count; ++index) {
        const auto& barrier = barriers[index];
        switch (barrier.type) {
        case RenderResourceBarrierType::kTransition:
            process_transition(
                context,
                barrier,
                shared_label,
                cache_actions,
                needs_completion_wait);
            break;
        case RenderResourceBarrierType::kAliasing:
            break;
        case RenderResourceBarrierType::kUnorderedAccess:
            synchronize_barrier_phase(
                context,
                barrier,
                kInvalidateL1,
                shared_label,
                cache_actions,
                needs_completion_wait);
            break;
        }
    }

    if (needs_completion_wait) {
        auto completion_cache_actions = cache_actions;
        if (orbis_render_context_recording_graphics(context)) {
            completion_cache_actions |= kWriteBackAndInvalidateL1L2;
            cache_actions = completion_cache_actions;
        }
        orbis_render_context_emit_transition_completion_wait(
            context, completion_cache_actions);
    }

    if (cache_actions != 0) {
        orbis_render_context_flush_transition_caches(
            context, cache_actions);
    }
}

}  // namespace rb4
