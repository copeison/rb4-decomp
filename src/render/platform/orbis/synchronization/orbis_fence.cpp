#include "render/platform/orbis/synchronization/orbis_fence.h"

#include <cstddef>
#include <cstdint>
#include <limits>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/synchronization/orbis_fence_commands.h"
#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

namespace {

constexpr const char* kFenceAllocationName = "PS4Fence";

struct OrbisFenceDispatch {
    void (*destruct)(OrbisFence& fence);
    void (*delete_fence)(OrbisFence& fence);
};

OrbisFenceDispatch kOrbisFenceDispatch{
    orbis_fence_destruct,
    orbis_fence_delete,
};

void set_fence_dispatch(OrbisFence& fence) {
    fence.implementation = &kOrbisFenceDispatch;
}

std::uint32_t* allocate_fence_value() {
    return static_cast<std::uint32_t*>(MemAlloc(
        sizeof(std::uint32_t), kFenceAllocationName, 4));
}

void release_fence_value(OrbisFence& fence) {
    if (auto* system = orbis_render_system_instance()) {
        orbis_defer_allocation_release(*system, fence.value);
    } else {
        MemFree(fence.value);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D85C0.
OrbisFence* orbis_create_fence() {
    auto* storage = operator new(sizeof(OrbisFence));
    auto* fence = reinterpret_cast<OrbisFence*>(storage);
    orbis_fence_construct(*fence);
    return fence;
}

// Reconstructed from eboot.elf at 0x8E1570.
void orbis_fence_construct(OrbisFence& fence) {
    set_fence_dispatch(fence);
    fence.value = nullptr;
    fence.sequence = 0;
    auto* value = allocate_fence_value();
    *value = 0;
    fence.value = value;
}

// Reconstructed from eboot.elf at 0x8E15F0.
void orbis_fence_destruct(OrbisFence& fence) {
    set_fence_dispatch(fence);
    release_fence_value(fence);
    fence.value = nullptr;
}

// Reconstructed from eboot.elf at 0x8E1640.
void orbis_fence_base_destruct(OrbisFence& fence) {
    release_fence_value(fence);
    fence.value = nullptr;
}

// Reconstructed from eboot.elf at 0x8E1680.
void orbis_fence_delete(OrbisFence& fence) {
    set_fence_dispatch(fence);
    release_fence_value(fence);
    MemFree(&fence);
}

// Reconstructed from eboot.elf at 0x8E16D0.
std::uint32_t orbis_fence_next_value(OrbisFence& fence) {
    auto sequence = fence.sequence;
    if (sequence == std::numeric_limits<std::uint32_t>::max()) {
        fence.sequence = 0;
        release_fence_value(fence);

        auto* value = allocate_fence_value();
        *value = 0;
        fence.value = value;
        sequence = fence.sequence;
    }

    ++sequence;
    fence.sequence = sequence;
    return sequence;
}

// Reconstructed from eboot.elf at 0x8EB730.
void orbis_render_context_signal_fence(
    OrbisRenderContext& context,
    OrbisFence& fence) {
    auto* address = fence.value;
    const auto value = orbis_fence_next_value(fence);
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
    const OrbisFence& fence) {
    const auto* address = fence.value;
    const auto value = fence.sequence;
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_emit_graphics_fence_wait(
            context, address, value);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_emit_compute_fence_wait(
            context, address, value);
    }
}

}  // namespace rb4
