#pragma once

namespace rb4 {

struct RenderContext;
struct RenderSystem;

void render_system_update_frame_phase_metrics(RenderSystem& system);
void render_system_prepare_global_frame_resources(RenderContext& context);

}  // namespace rb4
