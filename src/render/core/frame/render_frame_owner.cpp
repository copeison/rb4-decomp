#include "render/core/frame/render_frame_owner.h"

#include <limits>

namespace rb4 {

// Reconstructed from eboot.elf at 0x448730.
RenderExtent render_frame_owner_output_extent(const RenderFrameOwner& owner) {
    const auto targets = render_frame_owner_target_states(owner);
    if (targets.count == 0) {
        return {};
    }

    const auto& state = *targets.states[0];
    return {
        state.width,
        state.height,
    };
}

// Reconstructed from eboot.elf at 0x448760.
std::uint32_t render_frame_owner_draw_mode(const RenderFrameOwner& owner) {
    const auto targets = render_frame_owner_target_states(owner);
    return targets.count == 0
        ? std::numeric_limits<std::uint32_t>::max()
        : targets.states[0]->draw_mode;
}

// Reconstructed from eboot.elf at 0x4487C0.
std::uint32_t render_frame_owner_debug_view(const RenderFrameOwner& owner) {
    const auto targets = render_frame_owner_target_states(owner);
    return targets.count == 0
        ? std::numeric_limits<std::uint32_t>::max()
        : targets.states[0]->debug_view;
}

// Reconstructed from eboot.elf at 0x448780.
void render_frame_owner_set_draw_mode(
    RenderFrameOwner& owner,
    std::uint32_t mode) {
    const auto targets = render_frame_owner_target_states(owner);
    for (std::size_t index = 0; index < targets.count; ++index) {
        targets.states[index]->draw_mode = mode;
    }
}

// Reconstructed from eboot.elf at 0x4487E0.
void render_frame_owner_set_debug_view(
    RenderFrameOwner& owner,
    std::uint32_t view) {
    const auto targets = render_frame_owner_target_states(owner);
    for (std::size_t index = 0; index < targets.count; ++index) {
        targets.states[index]->debug_view = view;
    }
}

}  // namespace rb4
