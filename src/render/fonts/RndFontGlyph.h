#pragma once

#include <cstddef>

// One glyph of a font page: its character, three pixel metrics and its
// texture rectangle. Only the layout RndFontPage::SetGlyph (0x667380)
// writes is recovered; field names are not in the reference map.
struct RndFontGlyph {
    unsigned short mChar;
    // Padding that SetGlyph writes as zero; nothing reads it.
    unsigned short mPad;
    int mWidth;
    // The pixel offset a line starts at when this glyph begins it; the
    // typesetter copies it into RndTypesetter::Glyph::mBearing.
    int mBearing;
    // Copied into RndTypesetter::Glyph::mExtent.
    int mExtent;
    // Left, top, width and height, in texture coordinates. The width is
    // mWidth's and the height is the size's glyph height.
    float mUV[4];
};

static_assert(offsetof(RndFontGlyph, mWidth) == 4);
static_assert(offsetof(RndFontGlyph, mUV) == 16);
static_assert(sizeof(RndFontGlyph) == 32);
