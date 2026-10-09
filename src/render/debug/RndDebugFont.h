#pragma once

// The renderer's built-in debug font and the extended fonts listed in the
// rnd/font/extended_fonts config. Only the startup and shutdown entry points
// that Rnd::Init and Rnd::Terminate call are declared.
class RndDebugFont {
public:
    // Creates the "Debug" font from the built-in glyph bitmap.
    static void Init();  // 0x65C1A0
    static void Terminate();  // 0x65C790

    // Loads the fonts that the extended_fonts config enables. Rnd::Init
    // calls it last; the object placement follows the neighbouring Init.
    // Name not in the reference map.
    static void InitExtendedFonts();  // 0x65CAF0
    // Releases the extended fonts. Name not in the reference map.
    static void TerminateExtendedFonts();  // 0x65CE70
};
