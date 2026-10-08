#include "render/core/system/render_system.h"

namespace rb4 {

void render_system_platform_prepare_frame(
    RenderSystem& system,
    bool auxiliary_frame) {
    system.virtual_table->prepare_frame(&system, auxiliary_frame);
}

void render_system_platform_submit_frame(
    RenderSystem& system,
    RenderFrameOwnerList& frame_owners,
    bool auxiliary_frame) {
    system.virtual_table->submit_frame(
        &system, &frame_owners, auxiliary_frame);
}

std::uint32_t render_system_frame_phase(RenderSystem& system) {
    return system.virtual_table->frame_phase(&system);
}

}  // namespace rb4
