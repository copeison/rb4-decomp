#pragma once

namespace rb4 {

struct RenderSystem;
struct RenderContext;

void render_system_update_frame_phase_metrics(RenderSystem& system);
void* render_system_begin_gpu_frame_tracking(
    RenderSystem& system,
    RenderContext& context);
void render_system_end_gpu_frame_tracking(
    RenderSystem& system,
    RenderContext& context,
    void* gpu_frame_stat);
void render_system_finalize_primary_context(
    RenderSystem& system,
    RenderContext& context);

}  // namespace rb4
