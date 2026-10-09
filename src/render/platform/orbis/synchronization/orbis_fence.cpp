#include "render/platform/orbis/synchronization/orbis_fence.h"

#include "render/platform/orbis/synchronization/orbis_fence_commands.h"
#include "renderps4/system/PS4Fence.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8EB730.
void orbis_render_context_signal_fence(
    OrbisRenderContext& context,
    PS4Fence& fence) {
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
void orbis_render_context_wait_fence(
    OrbisRenderContext& context,
    const PS4Fence& fence) {
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

}  // namespace rb4
