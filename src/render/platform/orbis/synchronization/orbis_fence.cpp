#include "render/platform/orbis/synchronization/orbis_fence.h"

#include <cstddef>
#include <cstdint>
#include <limits>

#include "render/platform/orbis/synchronization/orbis_fence_adapters.h"
#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisFenceSize = 24;
constexpr const char* kFenceAllocationName = "PS4Fence";

void release_fence_value(OrbisFence& fence) {
    auto* value = orbis_fence_value_storage(fence);
    if (auto* system = current_orbis_render_system()) {
        orbis_defer_allocation_release(*system, value);
    } else {
        render_release(value);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D85C0.
OrbisFence* orbis_create_fence() {
    auto* storage = render_allocate(kOrbisFenceSize);
    auto* fence = reinterpret_cast<OrbisFence*>(storage);
    orbis_fence_construct(*fence);
    return fence;
}

// Reconstructed from eboot.elf at 0x8E1570.
void orbis_fence_construct(OrbisFence& fence) {
    auto* value = orbis_allocate_fence_value(
        sizeof(std::uint32_t), kFenceAllocationName, 4);
    *value = 0;
    orbis_fence_set_value_storage(fence, value);
}

// Reconstructed from eboot.elf at 0x8E15F0.
void orbis_fence_destruct(OrbisFence& fence) {
    release_fence_value(fence);
    orbis_fence_set_value_storage(fence, nullptr);
}

// Reconstructed from eboot.elf at 0x8E1640.
void orbis_fence_base_destruct(OrbisFence& fence) {
    release_fence_value(fence);
    orbis_fence_set_value_storage(fence, nullptr);
}

// Reconstructed from eboot.elf at 0x8E1680.
void orbis_fence_delete(OrbisFence& fence) {
    release_fence_value(fence);
    render_delete_fence(fence);
}

// Reconstructed from eboot.elf at 0x8E16D0.
std::uint32_t orbis_fence_next_value(OrbisFence& fence) {
    auto sequence = orbis_fence_sequence(fence);
    if (sequence == std::numeric_limits<std::uint32_t>::max()) {
        orbis_fence_set_sequence(fence, 0);
        release_fence_value(fence);

        auto* value = orbis_allocate_fence_value(
            sizeof(std::uint32_t), kFenceAllocationName, 4);
        *value = 0;
        orbis_fence_set_value_storage(fence, value);
        sequence = 0;
    }

    ++sequence;
    orbis_fence_set_sequence(fence, sequence);
    return sequence;
}

// Reconstructed from eboot.elf at 0x8EB730.
void orbis_render_context_signal_fence(
    OrbisRenderContext& context,
    OrbisFence& fence) {
    auto* address = orbis_fence_value_storage(fence);
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
    const auto* address = orbis_fence_value_storage(fence);
    const auto value = orbis_fence_sequence(fence);
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_emit_graphics_fence_wait(
            context, address, value);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_emit_compute_fence_wait(
            context, address, value);
    }
}

}  // namespace rb4
