#pragma once

namespace Hmx {

class Color {
public:
    Color() = default;
    constexpr Color(float r, float g, float b, float a = 1.0F)
        : red(r), green(g), blue(b), alpha(a) {}

    // Shared constant colors.
    static const Color& GetZero() {
        static const Color sZero(0.0F, 0.0F, 0.0F, 0.0F);
        return sZero;
    }
    static const Color& GetWhite() {
        static const Color sWhite(1.0F, 1.0F, 1.0F, 1.0F);
        return sWhite;
    }

    float red;
    float green;
    float blue;
    float alpha;
};

static_assert(sizeof(Color) == 16);

}  // namespace Hmx

// Converts an sRGB-encoded color to linear space; alpha is copied.
void GammaToLinear_sRGB(const Hmx::Color& gamma, Hmx::Color& linear);
