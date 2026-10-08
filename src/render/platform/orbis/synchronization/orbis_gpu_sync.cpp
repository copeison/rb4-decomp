#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"

#include <cstddef>
#include <_pthread.h>

#include "render/core/system/render_epoch.h"
#include "render/platform/orbis/synchronization/orbis_gpu_sync_adapters.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8D8100.
void orbis_render_system_wait_idle(OrbisRenderSystem& system) {
    orbis_wait_for_gpu_idle(system);
    orbis_release_retired_allocations(system);
    if (orbis_frame_is_active(system)) {
        orbis_flush_active_frame(system);
    }
    orbis_render_context_reset_active_frame(
        orbis_render_system_context(system));
}

// Reconstructed from eboot.elf at 0x8D8140.
void orbis_wait_for_gpu_idle(OrbisRenderSystem& system) {
    while (!orbis_render_context_submissions_complete(
        orbis_render_system_context(system))) {
        scePthreadYield();
        if (orbis_frame_is_active(system)) {
            orbis_flush_active_frame(system);
        }
    }
}

// Reconstructed from eboot.elf at 0x8D8200.
void orbis_release_retired_allocations(OrbisRenderSystem& system) {
    const auto frame = orbis_render_system_epoch(system);
    if (frame < 2) {
        return;
    }

    const auto completed_frame = frame - 2;
    orbis_lock_retired_allocations(system);
    std::size_t index = 0;
    while (index < orbis_retired_allocation_count(system)) {
        if (orbis_retired_allocation_frame(system, index) <= completed_frame) {
            orbis_release_retired_allocation(system, index);
            orbis_erase_retired_allocation(system, index);
        } else {
            ++index;
        }
    }
    orbis_unlock_retired_allocations(system);
}

// Reconstructed from eboot.elf at 0x8D83F0.
void orbis_defer_allocation_release(
    OrbisRenderSystem& system,
    void* allocation) {
    if (allocation == nullptr) {
        return;
    }

    orbis_lock_retired_allocations(system);
    orbis_enqueue_retired_allocation(
        system, allocation, current_render_epoch());
    orbis_unlock_retired_allocations(system);
}

// Reconstructed from eboot.elf at 0x8D84B0.
void orbis_release_all_retired_allocations(OrbisRenderSystem& system) {
    orbis_lock_retired_allocations(system);
    while (orbis_retired_allocation_count(system) != 0) {
        orbis_release_retired_allocation(system, 0);
        orbis_erase_retired_allocation(system, 0);
    }
    orbis_unlock_retired_allocations(system);
}

}  // namespace rb4
