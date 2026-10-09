#pragma once

class RndFont;
class RndTexture2D;

// The renderer's built-in debug font and the extended fonts listed in the
// rnd/font/extended_fonts config.
class RndDebugFont {
public:
    // Creates the "Debug" font from the built-in glyph bitmap.
    static void Init();  // 0x65C1A0
    static void Terminate();  // 0x65C790
    static RndFont* GetInstance();  // 0x65C7D0

    // Loads the fonts that the extended_fonts config enables for the system
    // language. Rnd::Init calls it last. Name not in the reference map.
    static void InitExtendedFonts();  // 0x65CAF0
    // Releases the extended fonts. Name not in the reference map.
    static void TerminateExtendedFonts();  // 0x65CE70

    // The glyphs of the printable ASCII characters and a final block glyph,
    // one bit per pixel, eight bytes to a 64-pixel row of the texture.
    static const unsigned char gBitmapData[96 * 8];  // 0x19BDD20

private:
    // Draws the bitmap into a 64 by 128 texture: white glyph pixels on
    // transparent black.
    static RndTexture2D* _CreateTexture();  // 0x65C4E0
};
