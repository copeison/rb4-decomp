#pragma once

namespace Hmx {
class Color;
}

class RndPixelCanvas;
class RndTexture1DResource;
class Vector2i;
template <class T>
class Waveform;
enum RndDefaultTextureType : int;
// The engine's texture wrap modes; the enumerators are not recovered.
enum RndTexWrapMode : int;

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

    // Bakes the waveform into a new 1D texture resource of the size, with
    // the wrap mode. The map passes the waveform by value. Not
    // reconstructed.
    static RndTexture1DResource* CreateWaveformFloatTextureResource(
        const Waveform<float>& waveform,
        RndTexWrapMode wrap,
        Vector2i size);  // 0x6AF510
};
