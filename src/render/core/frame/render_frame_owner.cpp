#include "render/core/frame/render_frame_owner.h"

#include <limits>

namespace rb4 {

// Reconstructed from eboot.elf at 0x448730.
RenderExtent render_frame_owner_output_extent(const RenderFrameOwner& owner) {
    const auto* target = render_frame_owner_primary_target(owner);
    if (target == nullptr) {
        return {};
    }

    return {
        render_target_width(*target),
        render_target_height(*target),
    };
}

// Reconstructed from eboot.elf at 0x448760.
std::uint32_t render_frame_owner_draw_mode(const RenderFrameOwner& owner) {
    const auto* target = render_frame_owner_primary_target(owner);
    return target == nullptr
        ? std::numeric_limits<std::uint32_t>::max()
        : render_target_draw_mode(*target);
}

// Reconstructed from eboot.elf at 0x4487C0.
std::uint32_t render_frame_owner_debug_view(const RenderFrameOwner& owner) {
    const auto* target = render_frame_owner_primary_target(owner);
    return target == nullptr
        ? std::numeric_limits<std::uint32_t>::max()
        : render_target_debug_view(*target);
}

// Reconstructed from eboot.elf at 0x448780.
void render_frame_owner_set_draw_mode(
    RenderFrameOwner& owner,
    std::uint32_t mode) {
    const auto target_count = render_frame_owner_target_count(owner);
    for (std::size_t index = 0; index < target_count; ++index) {
        auto* target = render_frame_owner_target_at(owner, index);
        if (target != nullptr) {
            render_target_set_draw_mode(*target, mode);
        }
    }
}

// Reconstructed from eboot.elf at 0x4487E0.
void render_frame_owner_set_debug_view(
    RenderFrameOwner& owner,
    std::uint32_t view) {
    const auto target_count = render_frame_owner_target_count(owner);
    for (std::size_t index = 0; index < target_count; ++index) {
        auto* target = render_frame_owner_target_at(owner, index);
        if (target != nullptr) {
            render_target_set_debug_view(*target, view);
        }
    }
}

}  // namespace rb4
