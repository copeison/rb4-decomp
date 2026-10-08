#pragma once

#include "render/core/frame/render_frame_owner_list.h"

namespace rb4 {

struct OrbisRenderSystem;

void orbis_render_system_submit_frame(
    OrbisRenderSystem& system,
    const RenderFrameOwnerList& back_buffers);

}  // namespace rb4
