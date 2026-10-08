#pragma once

#include <cstdint>

#include "render/platform/orbis/video/orbis_video_output.h"

namespace rb4 {

struct OrbisRenderSystem;
void orbis_configure_submit_thread(
    OrbisRenderSystem& system,
    OrbisSubmitThreadEntry entry,
    const char* name,
    std::uint32_t priority);
void render_free(void* allocation);

}  // namespace rb4
