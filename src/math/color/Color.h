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
    static const Color& GetBlack() {
        static const Color sBlack(0.0F, 0.0F, 0.0F, 1.0F);
        return sBlack;
    }
    static const Color& GetCyan() {
        static const Color sCyan(0.0F, 1.0F, 1.0F, 1.0F);
        return sCyan;
    }
    static const Color& GetOrange() {
        static const Color sOrange(1.0F, 0.5F, 0.0F, 1.0F);
        return sOrange;
    }
    static const Color& GetRed() {
        static const Color sRed(1.0F, 0.0F, 0.0F, 1.0F);
        return sRed;
    }
    static const Color& GetGreen() {
        static const Color sGreen(0.0F, 1.0F, 0.0F, 1.0F);
        return sGreen;
    }
    static const Color& GetBlue() {
        static const Color sBlue(0.0F, 0.0F, 1.0F, 1.0F);
        return sBlue;
    }
    static const Color& GetYellow() {
        static const Color sYellow(1.0F, 1.0F, 0.0F, 1.0F);
        return sYellow;
    }
    static const Color& GetMagenta() {
        static const Color sMagenta(1.0F, 0.0F, 1.0F, 1.0F);
        return sMagenta;
    }
    static const Color& GetViolet() {
        static const Color sViolet(0.5F, 0.0F, 1.0F, 1.0F);
        return sViolet;
    }
    static const Color& GetGrey() {
        static const Color sGrey(0.5F, 0.5F, 0.5F, 1.0F);
        return sGrey;
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
