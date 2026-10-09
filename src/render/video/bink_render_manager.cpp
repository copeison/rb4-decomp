#include "render/video/bink_render_manager.h"

#include "render/video/bink_render_manager_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x5F2B40.
void bink_render_manager_prepare_frame(
    BinkRenderManager& manager,
    RndContext& context) {
    bink_render_context_prepare(context);
    for (auto* video : manager.conversion_slots) {
        if (video != nullptr && video->conversion_pending) {
            bink_video_convert_frame(*video, context);
            video->conversion_pending = false;
        }
    }
}

}  // namespace rb4
