#pragma once

namespace rb4 {

// Linear-space conversion of an sRGB-encoded RGBA color; alpha is copied.
void color_srgb_to_linear(const float (&srgb)[4], float (&linear)[4]);

}  // namespace rb4
