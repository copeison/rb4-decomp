#pragma once

namespace Hmx {

class Color {
public:
    Color() = default;
    Color(float r, float g, float b, float a = 1.0F)
        : red(r), green(g), blue(b), alpha(a) {}

    float red;
    float green;
    float blue;
    float alpha;
};

static_assert(sizeof(Color) == 16);

}  // namespace Hmx

// Converts an sRGB-encoded color to linear space; alpha is copied.
void GammaToLinear_sRGB(const Hmx::Color& gamma, Hmx::Color& linear);
