#pragma once

namespace rb4 {

struct OrbisRenderSystem;

void orbis_render_system_wait_idle(OrbisRenderSystem& system);
void orbis_wait_for_gpu_idle(OrbisRenderSystem& system);
void orbis_release_retired_allocations(OrbisRenderSystem& system);
void orbis_defer_allocation_release(
    OrbisRenderSystem& system,
    void* allocation);
void orbis_release_all_retired_allocations(OrbisRenderSystem& system);

}  // namespace rb4
