#pragma once

#include <cstddef>

// One glyph of a font page: its character, three pixel metrics and its
// texture rectangle. Only the layout RndFontPage::SetGlyph (0x667380)
// writes is recovered; field names are not in the reference map.
struct RndFontGlyph {
    unsigned short mChar;
    unsigned short mUnknown2;  // Zero.
    int mWidth;
    int mUnknown8;
    int mUnknown12;
    float mUV[4];  // Left, top, right and bottom, in texture coordinates.
};

static_assert(offsetof(RndFontGlyph, mWidth) == 4);
static_assert(offsetof(RndFontGlyph, mUV) == 16);
static_assert(sizeof(RndFontGlyph) == 32);
