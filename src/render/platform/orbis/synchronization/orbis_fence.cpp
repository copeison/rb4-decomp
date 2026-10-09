#include "render/platform/orbis/synchronization/orbis_fence.h"
#include "renderps4/context/PS4Context.h"

#include "render/platform/orbis/synchronization/orbis_fence_commands.h"
#include "renderps4/system/PS4Fence.h"

using namespace rb4;

// Reconstructed from eboot.elf at 0x8EB730.
void PS4Context::_SignalFenceImpl(RndFence& baseFence) {
    auto& context = *this;
    auto& fence = static_cast<PS4Fence&>(baseFence);
    auto* address = fence.mLabel;
    const auto value = fence.NextValue();
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_emit_graphics_fence_signal(
            context, address, value);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_emit_compute_fence_signal(
            context, address, value);
    }
}

// Reconstructed from eboot.elf at 0x8EB7F0.
void PS4Context::_WaitFenceImpl(const RndFence& baseFence) {
    auto& context = *this;
    const auto& fence = static_cast<const PS4Fence&>(baseFence);
    const auto* address = fence.mLabel;
    const auto value = fence.mSequence;
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_emit_graphics_fence_wait(
            context, address, value);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_emit_compute_fence_wait(
            context, address, value);
    }
}
