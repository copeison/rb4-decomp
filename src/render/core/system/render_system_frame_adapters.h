#pragma once

namespace rb4 {

struct RenderSystem;

void render_system_prepare_frame(RenderSystem& system, bool auxiliary_frame);
void render_system_finish_frame(RenderSystem& system, bool auxiliary_frame);

}  // namespace rb4
