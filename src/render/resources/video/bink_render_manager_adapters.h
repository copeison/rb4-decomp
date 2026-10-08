#pragma once

namespace rb4 {

struct BinkRenderManager;
struct BinkVideoRenderObject;
struct RenderContext;

BinkRenderManager& bink_render_manager_instance();
void bink_render_context_prepare(RenderContext& context);
void bink_video_convert_frame(
    BinkVideoRenderObject& video,
    RenderContext& context);

}  // namespace rb4
