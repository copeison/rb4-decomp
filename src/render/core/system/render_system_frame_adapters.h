#pragma once

namespace rb4 {

struct RenderSystem;
struct RenderContext;

void render_system_update_frame_phase_metrics(RenderSystem& system);
void* render_system_begin_gpu_frame_tracking(
    RenderSystem& system,
    RenderContext& context);
void render_system_finish_frame(RenderSystem& system, bool auxiliary_frame);

}  // namespace rb4
