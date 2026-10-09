#include "render/platform/orbis/context/orbis_render_context.h"
#include "renderps4/context/PS4Context.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/buffers/orbis_transient_vertex_buffer.h"
#include "render/platform/orbis/context/orbis_render_context_adapters.h"
#include "renderps4/system/PS4Device.h"

using namespace rb4;

namespace rb4 {

namespace {

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
constexpr std::size_t kSubmissionCounterCount = 10;
constexpr std::size_t kGraphicsCommandContextOffset = 0x5728;
constexpr std::size_t kGraphicsCommandContextStride = 0xE888;
constexpr std::size_t kTransientVertexBufferOffset = 0x40DB8;
constexpr std::size_t kTransientVertexBufferBankStride = 0x540;

struct OrbisRenderContextRuntimePrefix {
    std::uint8_t reserved_0[9];
    bool compute_queues_disabled;
    std::uint8_t reserved_10[0x22876];
    volatile std::int32_t
        submission_counters[kOrbisFrameSlotCount][kSubmissionCounterCount];
    std::uint8_t reserved_228D0[0x1E4C0];
    std::size_t active_frame;
};

static_assert(
    offsetof(OrbisRenderContextRuntimePrefix, compute_queues_disabled) == 9);
static_assert(
    offsetof(OrbisRenderContextRuntimePrefix, submission_counters) ==
    0x22880);
static_assert(
    offsetof(OrbisRenderContextRuntimePrefix, active_frame) == 0x40D90);

}  // namespace

PS4Context* orbis_render_context_create(
    PS4Device& system) {
    auto* context = new PS4Context;
    system._InstallImmediateContext(&*context);
    return context;
}

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8E72B0.
PS4Context::PS4Context() : RndContext(false) {
    auto& context = *this;
    orbis_render_context_initialize_command_state(context);

    for (std::size_t slot = 0; slot < kOrbisComputeContextCount; ++slot) {
        orbis_render_context_construct_compute_slot(context, slot);
    }
    orbis_render_context_initialize_state_defaults(context);

    for (std::size_t bank = 0; bank < kOrbisFrameSlotCount; ++bank) {
        for (std::size_t format = 0;
             format < kOrbisTransientFormatCount;
             ++format) {
            orbis_transient_vertex_buffer_construct(
                orbis_render_context_transient_vertex_buffer(
                    context, bank, format));
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
                orbis_render_context_transient_vertex_buffer(
                    context, bank, format),
                static_cast<RndVertexType>(format),
                kTransientVertexCapacity);
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

namespace rb4 {

// Reconstructed from eboot.elf at 0x8E7AF0.
void orbis_render_context_create_gfx_contexts(
    PS4Context& context) {
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
    PS4Context& context) {
    orbis_render_context_initialize_timestamp_records(
        context, kTimestampBufferSize);
}

bool orbis_render_context_compute_queues_enabled(
    const PS4Context& context) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderContextRuntimePrefix*>(&context);
    return !runtime->compute_queues_disabled;
}

std::size_t orbis_render_context_active_frame(
    const PS4Context& context) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderContextRuntimePrefix*>(&context);
    return runtime->active_frame;
}

OrbisTransientVertexBuffer& orbis_render_context_transient_vertex_buffer(
    PS4Context& context,
    std::size_t frame,
    std::size_t format) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&context);
    return *reinterpret_cast<OrbisTransientVertexBuffer*>(
        bytes + kTransientVertexBufferOffset +
        frame * kTransientVertexBufferBankStride +
        format * sizeof(OrbisTransientVertexBuffer));
}

OrbisRenderCommandContext& orbis_active_render_command_context(
    PS4Context& context) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&context);
    return *reinterpret_cast<OrbisRenderCommandContext*>(
        bytes + kGraphicsCommandContextOffset +
        orbis_render_context_active_frame(context) *
            kGraphicsCommandContextStride);
}

bool orbis_render_context_submissions_complete(
    const PS4Context& context) {
    return orbis_render_context_frame_submissions_complete(
        context, orbis_render_context_active_frame(context));
}

bool orbis_render_context_frame_submissions_complete(
    const PS4Context& context,
    std::size_t frame) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderContextRuntimePrefix*>(&context);
    for (std::size_t index = 0; index < kSubmissionCounterCount; ++index) {
        if (runtime->submission_counters[frame][index] != 0) {
            return false;
        }
    }
    return true;
}

void orbis_render_context_mark_compute_completion_pending(
    PS4Context& context,
    std::size_t frame,
    std::size_t slot) {
    auto* runtime =
        reinterpret_cast<OrbisRenderContextRuntimePrefix*>(&context);
    runtime->submission_counters[frame][slot + 1] = 1;
}

void orbis_render_context_mark_gfx_completion_pending(
    PS4Context& context,
    std::size_t frame) {
    auto* runtime =
        reinterpret_cast<OrbisRenderContextRuntimePrefix*>(&context);
    runtime->submission_counters[frame][0] = 1;
}

void orbis_render_context_set_active_frame(
    PS4Context& context,
    std::size_t frame) {
    auto* runtime =
        reinterpret_cast<OrbisRenderContextRuntimePrefix*>(&context);
    runtime->active_frame = frame;
}

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8E8070.
PS4Context::~PS4Context() {
    auto& context = *this;
    orbis_render_context_release_label_pool(context);
    orbis_render_context_release_timestamp_pool(context);
    for (std::size_t bank = kOrbisFrameSlotCount; bank-- > 0;) {
        for (std::size_t format = kOrbisTransientFormatCount; format-- > 0;) {
            orbis_transient_vertex_buffer_destruct(
                orbis_render_context_transient_vertex_buffer(
                    context, bank, format));
        }
    }
    for (std::size_t slot = kOrbisComputeContextCount; slot-- > 0;) {
        orbis_render_context_destruct_compute_slot(context, slot);
    }
    orbis_render_context_destruct_command_state(context);
    orbis_render_context_destruct_base(context);
}

namespace rb4 {

// Reconstructed from eboot.elf at 0x8E82D0.
void orbis_render_context_submit_frame(PS4Context& context) {
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
void orbis_render_context_reset_active_frame(PS4Context& context) {
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
        orbis_transient_vertex_buffer_reset(
            orbis_render_context_transient_vertex_buffer(
                context, frame, format));
    }
    orbis_render_context_emit_default_control_state(context, frame);
}

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8EA2A0, 0x8EA2B0, 0x8EA2C0, and 0x8EA730:
// the PS4 context ignores these states.
void PS4Context::_SetDepthClipEnabledImpl(bool) {}
void PS4Context::_SetDepthBiasEnabledImpl(bool) {}
void PS4Context::_SetThickLinesImpl(bool) {}
void PS4Context::_DrawIndirectImpl(RndPrimitive, const RndComputeBuffer&) {}
