#pragma once

namespace rb4 {

struct RenderSystem;
struct RenderContext;

void render_system_update_frame_phase_metrics(RenderSystem& system);
void render_system_prepare_frame_resources(
    RenderSystem& system,
    RenderContext& context);
void render_system_finalize_primary_context_resources(
    RenderSystem& system,
    RenderContext& context);

}  // namespace rb4
