#pragma once

#include <cstdint>

namespace rb4 {

struct RenderSystem;

std::uint64_t render_epoch(const RenderSystem& system);
std::uint64_t current_render_epoch();
void advance_render_epoch(RenderSystem& system);

}  // namespace rb4
