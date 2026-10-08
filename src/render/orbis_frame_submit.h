#pragma once

#include <vector>

namespace rb4 {

struct OrbisRenderObject;
struct OrbisRenderSystem;

void orbis_render_system_submit_frame(
    OrbisRenderSystem& system,
    const std::vector<OrbisRenderObject*>& objects);

}  // namespace rb4
