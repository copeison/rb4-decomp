#pragma once

#include <cstdint>

namespace rb4 {

constexpr std::uint32_t kInvalidRenderDebugMode = 0xFFFFFFFFu;

const char* render_draw_mode_name(std::uint32_t mode);
std::uint32_t render_draw_mode_from_name(const char* name);
const char* render_debug_view_name(std::uint32_t view);
std::uint32_t render_debug_view_from_name(const char* name);

}  // namespace rb4
