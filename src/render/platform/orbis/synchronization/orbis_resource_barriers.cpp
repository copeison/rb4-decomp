#include "render/platform/orbis/synchronization/orbis_resource_barriers.h"
#include "renderps4/context/PS4Context.h"

#include <cstdint>

#include "render/platform/orbis/synchronization/orbis_resource_barriers_adapters.h"
#include "render/platform/orbis/synchronization/orbis_resource_sync.h"

using namespace rb4;

namespace rb4 {

namespace {

constexpr std::uint32_t kInvalidateL1 = 0x10;
constexpr std::uint32_t kWriteBackAndInvalidateL1L2 = 0x38;

bool is_state(
    RndResourceState state,
    RndResourceState expected) {
    return state == expected;
}

bool is_write_destination(RndResourceState state) {
    return is_state(state, RndResourceState::kRenderTarget) ||
        is_state(state, RndResourceState::kUnorderedAccess) ||
        is_state(state, RndResourceState::kDepthWrite) ||
        is_state(state, RndResourceState::kStreamOutput) ||
        is_state(state, RndResourceState::kCopyDestination) ||
        is_state(state, RndResourceState::kResolveDestination);
}

std::uint32_t destination_cache_actions(RndResourceState state) {
    if (is_state(state, RndResourceState::kUnorderedAccess) ||
        is_state(state, RndResourceState::kStreamOutput) ||
        is_state(state, RndResourceState::kCopyDestination)) {
        return kWriteBackAndInvalidateL1L2;
    }
    return 0;
}

void synchronize_barrier_phase(
    PS4Context& context,
    const RndResourceBarrier& barrier,
    std::uint32_t barrier_cache_actions,
    volatile std::uint32_t*& shared_label,
    std::uint32_t& cache_actions,
    bool& needs_completion_wait) {
    switch (barrier.mPhase) {
    case RndResourceBarrierPhase::kImmediate:
        cache_actions |= barrier_cache_actions;
        needs_completion_wait = true;
        break;
    case RndResourceBarrierPhase::kBegin:
        orbis_render_context_signal_resource(
            context, barrier.mResource, shared_label);
        cache_actions |= barrier_cache_actions;
        break;
    case RndResourceBarrierPhase::kEnd:
        orbis_render_context_wait_for_resource(
            context, barrier.mResource);
        break;
    }
}

void resolve_texture_metadata(
    PS4Context& context,
    const RndResourceBarrier& barrier,
    bool resolve_depth,
    bool& needs_completion_wait) {
    if (resolve_depth) {
        orbis_render_context_resolve_depth_metadata(
            context, barrier.mResource, barrier.mSubresource);
    } else {
        orbis_render_context_resolve_color_metadata(
            context, barrier.mResource, barrier.mSubresource);
    }

    volatile std::uint32_t* metadata_label = nullptr;
    if (orbis_render_context_recording_compute(context)) {
        const auto compute_queue =
            orbis_render_context_active_compute_queue(context);
        orbis_render_context_select_graphics(context);
        orbis_render_context_signal_resource(
            context, barrier.mResource, metadata_label);
        orbis_render_context_select_compute(context, compute_queue);

        if (barrier.mPhase != RndResourceBarrierPhase::kBegin) {
            orbis_render_context_wait_for_resource(
                context, barrier.mResource);
        }
    } else if (barrier.mPhase == RndResourceBarrierPhase::kBegin) {
        orbis_render_context_signal_resource(
            context, barrier.mResource, metadata_label);
    } else {
        needs_completion_wait = true;
    }
}

void process_transition(
    PS4Context& context,
    const RndResourceBarrier& barrier,
    volatile std::uint32_t*& shared_label,
    std::uint32_t& cache_actions,
    bool& needs_completion_wait) {
    if (barrier.mBefore == barrier.mAfter) {
        return;
    }

    if (is_state(
            barrier.mAfter,
            RndResourceState::kRenderTarget)) {
        orbis_render_context_wait_for_render_target(
            context, barrier.mResource);
    }

    const auto barrier_cache_actions =
        destination_cache_actions(barrier.mAfter);

    if (is_state(
            barrier.mBefore,
            RndResourceState::kRenderTarget)) {
        if (barrier.mPhase == RndResourceBarrierPhase::kEnd) {
            orbis_render_context_wait_for_resource(
                context, barrier.mResource);
        } else {
            resolve_texture_metadata(
                context, barrier, false, needs_completion_wait);
        }
        return;
    }

    if (is_state(
            barrier.mBefore,
            RndResourceState::kDepthWrite)) {
        if (barrier.mPhase == RndResourceBarrierPhase::kEnd) {
            orbis_render_context_wait_for_resource(
                context, barrier.mResource);
        } else {
            resolve_texture_metadata(
                context, barrier, true, needs_completion_wait);
        }
        return;
    }

    if (is_state(
            barrier.mBefore,
            RndResourceState::kResolveDestination)) {
        return;
    }

    const bool source_requires_barrier =
        is_state(
            barrier.mBefore,
            RndResourceState::kUnorderedAccess) ||
        is_state(
            barrier.mBefore,
            RndResourceState::kStreamOutput) ||
        is_state(
            barrier.mBefore,
            RndResourceState::kCopyDestination);
    if (source_requires_barrier || is_write_destination(barrier.mAfter)) {
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

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8EAA10.
void PS4Context::_ResourceBarrierImpl(unsigned long barrier_count, const RndResourceBarrier* barriers) {
    auto& context = *this;
    volatile std::uint32_t* shared_label = nullptr;
    std::uint32_t cache_actions = 0;
    bool needs_completion_wait = false;

    for (std::size_t index = 0; index < barrier_count; ++index) {
        const auto& barrier = barriers[index];
        switch (barrier.mType) {
        case RndResourceBarrierType::kTransition:
            process_transition(
                context,
                barrier,
                shared_label,
                cache_actions,
                needs_completion_wait);
            break;
        case RndResourceBarrierType::kAliasing:
            break;
        case RndResourceBarrierType::kUnorderedAccess:
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
