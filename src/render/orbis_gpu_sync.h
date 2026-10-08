#pragma once

namespace rb4 {

struct OrbisRenderSystem;

void orbis_render_system_wait_idle(OrbisRenderSystem& system);
void orbis_wait_for_gpu_idle(OrbisRenderSystem& system);
void orbis_release_retired_allocations(OrbisRenderSystem& system);

}  // namespace rb4
