#pragma once

#include <cstdint>

namespace rb4 {

struct RenderContext;

void render_context_delete(RenderContext& context);

void render_context_begin_frame(
    RenderContext& context,
    std::uint32_t activation_flags);

}  // namespace rb4
