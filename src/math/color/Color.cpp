#include "math/color/Color.h"

#include <cmath>

namespace {

float GammaToLinearChannel_sRGB(float value) {
    constexpr float kLinearThreshold = 0.04045F;
    if (kLinearThreshold < value) {
        return std::pow((value + 0.055F) * (1.0F / 1.055F), 2.4F);
    }
    return value * (1.0F / 12.92F);
}

}  // namespace

// Reconstructed from eboot.elf at 0x211A70.
void GammaToLinear_sRGB(const Hmx::Color& gamma, Hmx::Color& linear) {
    linear.red = GammaToLinearChannel_sRGB(gamma.red);
    linear.green = GammaToLinearChannel_sRGB(gamma.green);
    linear.blue = GammaToLinearChannel_sRGB(gamma.blue);
    linear.alpha = gamma.alpha;
}
