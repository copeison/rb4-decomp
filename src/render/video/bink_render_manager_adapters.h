#pragma once

class RndContext;

namespace rb4 {

struct BinkRenderManager;
struct BinkVideoRenderObject;

BinkRenderManager& bink_render_manager_instance();
void bink_render_context_prepare(RndContext& context);
void bink_video_convert_frame(
    BinkVideoRenderObject& video,
    RndContext& context);

}  // namespace rb4
