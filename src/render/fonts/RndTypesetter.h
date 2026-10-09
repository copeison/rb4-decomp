#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "utl/containers/FixedVector.h"
#include "utl/text/Symbol.h"

class RndFont;

// How text is fitted to its box. The enumerators are not recovered.
enum RndTextFitMode : int;

// Lays wide text out in glyphs (render/RndTypesetter.o): fonts by style,
// markup, wrapping and alignment. Not reconstructed; only the members the
// debug text drawing uses are declared, and their field names are not in
// the reference map.
class RndTypesetter {
public:
    // One font of a style.
    struct StyleFont {
        RndFont* mFont;
        int mUnknown8;
    };

    // A named set of fonts, one per style size.
    struct Style {
        Symbol mName;
        int mUnknown8;
        FixedVector<StyleFont, 5> mFonts;
    };

    // What to lay out and how.
    struct Params {
        const unsigned short* mText;
        const Style* mStyles;
        unsigned long mNumStyles;
        int mUnknown24;
        int mUnknown28;
        int mUnknown32;
        RndTextFitMode mFitMode;
        // The box in font pixels; the height is used by fit mode 4.
        int mMaxWidth;
        int mMaxHeight;
        int mUnknown48;
        bool mMarkup;  // The text holds markup tags.
        unsigned char mUnknown56[24];
        Symbol mUnknown80;
    };

    // One laid-out glyph, in font pixels from the text's origin. The
    // texture rectangle's bottom maps to mMin.y.
    struct Glyph {
        Vector2i mMin;
        Vector2i mMax;
        float mUV[4];  // Left, top, width and height.
        unsigned int mStyle;  // An index into the params' styles.
        unsigned int mPage;   // An index into the font size's pages.
        unsigned char mUnknown40[24];
    };

    // The glyphs, in storage the caller provides, and the text's extent.
    struct Result {
        int mStyleSize;  // The style font used; -1 until laid out.
        Glyph* mGlyphs;
        unsigned long mNumGlyphs;
        unsigned long mCapacity;
        unsigned char mUnknown32[48];
        Vector2i mMin;
        Vector2i mMax;
        Vector2i mEnd;  // Where the next glyph would go.
    };

    // The glyphs the text has, counting markup tags as glyphs unless
    // `markup` is set. Whitespace does not count.
    static unsigned long CalcNumGlyphs(const unsigned short* text, bool markup);  // 0x67D920
    // The glyphs a result needs: three more when the fit mode is 2.
    static unsigned long CalcResultGlyphsCapacity(
        unsigned long numGlyphs,
        unsigned long numIcons,
        RndTextFitMode fitMode);  // 0x67E570
    static void ProcessText(const Params& params, Result& result);  // 0x67E590
};

static_assert(sizeof(RndTypesetter::StyleFont) == 16);
static_assert(offsetof(RndTypesetter::Style, mFonts) == 16);
static_assert(sizeof(RndTypesetter::Style) == 120);
static_assert(offsetof(RndTypesetter::Params, mUnknown28) == 28);
static_assert(offsetof(RndTypesetter::Params, mFitMode) == 36);
static_assert(offsetof(RndTypesetter::Params, mMaxWidth) == 40);
static_assert(offsetof(RndTypesetter::Params, mUnknown48) == 48);
static_assert(offsetof(RndTypesetter::Params, mMarkup) == 52);
static_assert(offsetof(RndTypesetter::Params, mUnknown80) == 80);
static_assert(sizeof(RndTypesetter::Params) == 88);
static_assert(offsetof(RndTypesetter::Glyph, mUV) == 16);
static_assert(offsetof(RndTypesetter::Glyph, mStyle) == 32);
static_assert(offsetof(RndTypesetter::Glyph, mPage) == 36);
static_assert(sizeof(RndTypesetter::Glyph) == 64);
static_assert(offsetof(RndTypesetter::Result, mGlyphs) == 8);
static_assert(offsetof(RndTypesetter::Result, mNumGlyphs) == 16);
static_assert(offsetof(RndTypesetter::Result, mMin) == 80);
static_assert(offsetof(RndTypesetter::Result, mEnd) == 96);
static_assert(sizeof(RndTypesetter::Result) == 104);
