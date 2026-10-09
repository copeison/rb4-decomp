#pragma once

namespace Hmx {
class Color;
}

class RndPixelCanvas;
enum RndDefaultTextureType : int;

// Texture helpers shared by the renderer.
class RndTextureUtl {
public:
    // Reconstructed from eboot.elf at 0x6AC700.
    static const char* GetDefaultTextureName(RndDefaultTextureType type);

    // Reconstructed from eboot.elf at 0x6AED00. Pixels in alternating cells
    // of `cellSize` texels in every dimension take the primary and secondary
    // colours; the cell at the origin is secondary. Rows are walked to the
    // height, not the width. Name not in the
    // reference map, whose older build has the fixed-size
    // RndDefaults::_FillTextureCanvasCheckerboard(RndPixelCanvas&,
    // Hmx::Color const&, Hmx::Color const&) instead.
    static void FillCheckerboard(
        RndPixelCanvas& canvas,
        const Hmx::Color& primary,
        const Hmx::Color& secondary,
        int cellSize);
};
