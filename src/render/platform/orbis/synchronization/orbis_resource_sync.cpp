#include "render/platform/orbis/synchronization/orbis_resource_sync.h"

#include "render/platform/orbis/synchronization/orbis_resource_sync_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kResourceReadyValue = 1;

}  // namespace

// Reconstructed from eboot.elf at 0x8EB3E0.
void orbis_render_context_signal_resource(
    OrbisRenderContext& context,
    const void* resource,
    volatile std::uint32_t*& shared_label) {
    if (shared_label == nullptr) {
        shared_label =
            orbis_render_context_allocate_resource_label(context);
        *shared_label = 0;

        if (orbis_render_context_recording_graphics(context)) {
            orbis_render_context_emit_graphics_resource_signal(
                context, shared_label, kResourceReadyValue);
        } else if (orbis_render_context_recording_compute(context)) {
            orbis_render_context_emit_compute_resource_signal(
                context, shared_label, kResourceReadyValue);
        }
    }

    const OrbisResourceSignal signal = {
        resource,
        shared_label,
        current_render_epoch(),
    };
    orbis_render_context_track_resource_signal(context, signal);
}

// Reconstructed from eboot.elf at 0x8EB590.
void orbis_render_context_wait_for_resource(
    OrbisRenderContext& context,
    const void* resource) {
    auto* signal =
        orbis_render_context_find_resource_signal(context, resource);
    if (signal == nullptr) {
        return;
    }

    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_emit_graphics_resource_wait(
            context, signal->label, kResourceReadyValue);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_emit_compute_resource_wait(
            context, signal->label, kResourceReadyValue);
    }

    orbis_render_context_remove_resource_signal_group(
        context, signal->label);
}

}  // namespace rb4
