#pragma once

#include <cstdint>

namespace rb4 {

void render_frame_phase_callbacks(bool enabled);
void render_set_partial_frame_phase(std::uint64_t phase);

}  // namespace rb4
