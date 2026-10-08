#pragma once

#include <cstdint>

#include "render_system_frame_adapters.h"

namespace rb4 {

struct RenderExtent {
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    bool empty() const {
        return width == 0 || height == 0;
    }
};

RenderExtent render_frame_owner_output_extent(const RenderFrameOwner& owner);
std::uint32_t render_frame_owner_draw_mode(const RenderFrameOwner& owner);
std::uint32_t render_frame_owner_debug_view(const RenderFrameOwner& owner);

}  // namespace rb4
