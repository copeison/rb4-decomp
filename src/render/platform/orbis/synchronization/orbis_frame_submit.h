#pragma once

#include <vector>

namespace rb4 {

struct OrbisBackBuffer;
struct OrbisRenderSystem;

void orbis_render_system_submit_frame(
    OrbisRenderSystem& system,
    const std::vector<OrbisBackBuffer*>& back_buffers);

}  // namespace rb4
