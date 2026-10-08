#include "render/platform/orbis/context/orbis_render_context.h"

#include <cstddef>

#include "render/platform/orbis/context/orbis_render_context_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisRenderContextSize = 0x44890;
constexpr std::size_t kTransientVertexCapacity = 0x40000;
constexpr std::size_t kCueSlotCount = 64;
constexpr std::size_t kCueHeapBytesPerSlot = 39872;
constexpr std::size_t kDrawCommandBufferSize = 32 * 1024 * 1024;
constexpr std::size_t kResourceBufferSize = 2 * 1024 * 1024;
constexpr std::size_t kConstantUpdateBufferSize = 4 * 1024 * 1024;
constexpr std::size_t kScratchBufferSize = 4 * 1024 * 1024;
constexpr std::size_t kComputeCommandBufferSize = 0x3FFFFC;
constexpr std::size_t kComputeQueueRingSize = 4096;
constexpr std::size_t kComputeQueueRingAlignment = 256;
constexpr std::size_t kTimestampBufferSize = 0x2000;
constexpr std::size_t kInitialLabelCapacity = 32;
constexpr std::size_t kHighPriorityComputeContextCount = 3;

struct OrbisRenderContextRuntimePrefix {
    std::uint8_t reserved_0[0x40D90];
    std::size_t active_frame;
};

static_assert(
    offsetof(OrbisRenderContextRuntimePrefix, active_frame) == 0x40D90);

}  // namespace

OrbisRenderContext* orbis_render_context_create(
    OrbisRenderSystem& system) {
    auto* storage = render_allocate(kOrbisRenderContextSize);
    auto* context = static_cast<OrbisRenderContext*>(storage);
    orbis_render_context_construct(*context);
    render_system_set_render_context(system, *context);
    return context;
}

// Reconstructed from eboot.elf at 0x8E72B0.
void orbis_render_context_construct(OrbisRenderContext& context) {
    orbis_render_context_construct_base(context);
    orbis_render_context_install_vtable(context);
    orbis_render_context_initialize_command_state(context);

    for (std::size_t slot = 0; slot < kOrbisComputeContextCount; ++slot) {
        orbis_render_context_construct_compute_slot(context, slot);
    }
    orbis_render_context_initialize_state_defaults(context);

    for (std::size_t bank = 0; bank < kOrbisFrameSlotCount; ++bank) {
        for (std::size_t format = 0;
             format < kOrbisTransientFormatCount;
             ++format) {
            orbis_transient_vertex_buffer_construct(context, bank, format);
        }
    }
    orbis_render_context_initialize_allocation_map(context);
    orbis_render_context_create_gfx_contexts(context);
    orbis_render_context_create_gpu_timestamp_pool(context);

    for (std::size_t bank = 0; bank < kOrbisFrameSlotCount; ++bank) {
        for (std::size_t format = 0;
             format < kOrbisTransientFormatCount;
             ++format) {
            orbis_transient_vertex_buffer_initialize(
                context, bank, format, kTransientVertexCapacity);
        }
    }

    if (!orbis_render_context_compute_queues_enabled(context)) {
        return;
    }

    orbis_render_context_initialize_compute_queue(
        context, 0, 1, 1, kComputeQueueRingSize, kComputeQueueRingAlignment);
    orbis_render_context_initialize_compute_queue(
        context, 1, 0, 0, kComputeQueueRingSize, kComputeQueueRingAlignment);
    for (std::size_t slot = 0; slot < kOrbisComputeContextCount; ++slot) {
        orbis_render_context_initialize_compute_context(
            context, slot, kCueSlotCount, kComputeCommandBufferSize);
    }
    orbis_render_context_initialize_label_pool(
        context, kInitialLabelCapacity);
}

// Reconstructed from eboot.elf at 0x8E7AF0.
void orbis_render_context_create_gfx_contexts(
    OrbisRenderContext& context) {
    const auto cue_heap_size = kCueSlotCount * kCueHeapBytesPerSlot;
    for (std::size_t slot = 0; slot < kOrbisFrameSlotCount; ++slot) {
        orbis_render_context_initialize_gfx_slot(
            context,
            slot,
            cue_heap_size,
            kDrawCommandBufferSize,
            kResourceBufferSize,
            kConstantUpdateBufferSize,
            kScratchBufferSize);
    }
}

// Reconstructed from eboot.elf at 0x8E7DF0.
void orbis_render_context_create_gpu_timestamp_pool(
    OrbisRenderContext& context) {
    orbis_render_context_initialize_timestamp_records(
        context, kTimestampBufferSize);
}

std::size_t orbis_render_context_active_frame(
    const OrbisRenderContext& context) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderContextRuntimePrefix*>(&context);
    return runtime->active_frame;
}

void orbis_render_context_set_active_frame(
    OrbisRenderContext& context,
    std::size_t frame) {
    auto* runtime =
        reinterpret_cast<OrbisRenderContextRuntimePrefix*>(&context);
    runtime->active_frame = frame;
}

// Reconstructed from eboot.elf at 0x8E8070.
void orbis_render_context_destruct(OrbisRenderContext& context) {
    orbis_render_context_release_label_pool(context);
    orbis_render_context_release_timestamp_pool(context);
    for (std::size_t bank = kOrbisFrameSlotCount; bank-- > 0;) {
        for (std::size_t format = kOrbisTransientFormatCount; format-- > 0;) {
            orbis_transient_vertex_buffer_destruct(context, bank, format);
        }
    }
    for (std::size_t slot = kOrbisComputeContextCount; slot-- > 0;) {
        orbis_render_context_destruct_compute_slot(context, slot);
    }
    orbis_render_context_destruct_command_state(context);
    orbis_render_context_destruct_base(context);
}

// Reconstructed from eboot.elf at 0x8E82B0.
void orbis_render_context_delete(OrbisRenderContext& context) {
    orbis_render_context_destruct(context);
    render_free(&context);
}

// Reconstructed from eboot.elf at 0x8E82D0.
void orbis_render_context_submit_frame(OrbisRenderContext& context) {
    const auto frame = orbis_render_context_active_frame(context);
    orbis_render_context_emit_end_of_frame_event(context, frame);

    for (std::size_t slot = 0;
         slot < kOrbisComputeContextsPerFrame;
         ++slot) {
        orbis_render_context_mark_compute_completion_pending(
            context, frame, slot);
        orbis_render_context_emit_compute_completion(context, frame, slot);

        const auto queue =
            slot < kHighPriorityComputeContextCount ? 0U : 1U;
        orbis_render_context_submit_compute(context, frame, slot, queue);
    }

    orbis_render_context_mark_gfx_completion_pending(context, frame);
    orbis_render_context_emit_gfx_completion(context, frame);
    orbis_render_context_submit_gfx(context, frame);
    orbis_render_context_set_active_frame(
        context, (frame + 1) % kOrbisFrameSlotCount);
}

// Reconstructed from eboot.elf at 0x8E8450.
void orbis_render_context_reset_active_frame(OrbisRenderContext& context) {
    const auto frame = orbis_render_context_active_frame(context);
    orbis_render_context_reset_gfx_slot(context, frame);
    orbis_render_context_initialize_gfx_hardware_state(context, frame);
    orbis_render_context_clear_frame_draw_count(context, frame);

    if (orbis_render_context_compute_queues_enabled(context)) {
        for (std::size_t slot = 0;
             slot < kOrbisComputeContextsPerFrame;
             ++slot) {
            orbis_render_context_reset_compute_slot(context, frame, slot);
        }
    }

    orbis_render_context_initialize_frame_command_state(context, frame);
    for (std::size_t format = 0;
         format < kOrbisTransientFormatCount;
         ++format) {
        orbis_transient_vertex_buffer_reset(context, frame, format);
    }
    orbis_render_context_emit_default_control_state(context, frame);
}

}  // namespace rb4
