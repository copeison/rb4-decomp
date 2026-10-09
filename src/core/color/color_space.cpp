#include "core/color/color_space.h"

#include <cmath>

namespace rb4 {

namespace {

float srgb_channel_to_linear(float value) {
    constexpr float kLinearThreshold = 0.04045F;
    if (kLinearThreshold < value) {
        return std::pow((value + 0.055F) * (1.0F / 1.055F), 2.4F);
    }
    return value * (1.0F / 12.92F);
}

}  // namespace

// Reconstructed from eboot.elf at 0x211A70.
void color_srgb_to_linear(const float (&srgb)[4], float (&linear)[4]) {
    linear[0] = srgb_channel_to_linear(srgb[0]);
    linear[1] = srgb_channel_to_linear(srgb[1]);
    linear[2] = srgb_channel_to_linear(srgb[2]);
    linear[3] = srgb[3];
}

}  // namespace rb4
