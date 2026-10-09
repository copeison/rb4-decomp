#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "render/fonts/RndTextEnums.h"
#include "utl/containers/BufVector.h"
#include "utl/containers/FixedVector.h"
#include "utl/text/Symbol.h"

class GameObject;
class RndFont;
struct RndFontGlyph;

// Lays wide text out in glyphs (render/RndTypesetter.o): fonts by style and
// size, markup tags, word wrap, truncation, shrinking, alignment and
// justification. Positions are in font pixels from the text's origin, with
// y growing up: the first line's top is at zero and each line lies below the
// last. Field names are not in the reference map.
class RndTypesetter {
public:
    // One size's font of a style. Name not in the reference map.
    struct StyleFont {
        RndFont* mFont;
        int mVerticalOffset;  // Added to a glyph's top in styles past the first.
    };

    // A named set of fonts, one per style size. Markup's <style=name>
    // switches to a style by name. The second half of a style list holds the
    // fallbacks for the first half, used with the extended fonts.
    struct Style {
        Symbol mName;
        // Where a style past the first sits within its line.
        RndTextAlignment mAlignment;
        FixedVector<StyleFont, 5> mFonts;
    };

    // An icon markup's <icon=name> inserts, as a blank glyph of its width.
    struct Icon {
        Symbol mName;
        int mWidth;
        int mVerticalOffset;  // Added to the icon's center.
    };

    // What to lay out and how.
    struct Params {
        const unsigned short* mText;
        const Style* mStyles;
        unsigned long mNumStyles;
        RndTextCapitalization mCapitalization;
        RndTextAlignment mAlignment;
        RndTextJustification mJustification;
        RndTextFitMode mFitMode;
        // The box in font pixels; nonpositive values impose no limit.
        int mMaxWidth;
        int mMaxHeight;
        RndFontStyleSize mStyleSize;  // The first size tried.
        bool mMarkup;                 // The text holds markup tags.
        const Icon* mIcons;
        unsigned long mNumIcons;
        // Name the text in error messages.
        GameObject* mObject;
        Symbol mToken;
    };

    // One laid-out glyph. The texture rectangle's bottom maps to mMin.y.
    // Name not in the reference map.
    struct Glyph {
        Vector2i mMin;
        Vector2i mMax;
        float mUV[4];          // Left, top, width and height.
        unsigned int mStyle;   // An index into the params' styles; -1 for icons.
        unsigned int mPage;    // An index into the font size's pages; -1 for icons.
        int mLineBottom;       // The line's extent, by the first style's font.
        int mLineTop;
        unsigned int mCharIndex;  // Into the text; -1 for the truncation periods.
        bool mBreakBefore;        // The line may wrap before this glyph.
        // The font glyph's left bearing, which a line's first glyph starts
        // at, and its right edge from mMin.x.
        int mBearing;
        int mExtent;
    };

    // How many glyphs of a style use a page of its font. Name not in the
    // reference map.
    struct PageGlyphs {
        unsigned long mStyle;
        unsigned long mPage;
        unsigned long mNumGlyphs;
    };

    // Where an icon went. Its glyph index is kept in mPosition.x until the
    // result is finalized. Name not in the reference map.
    struct IconResult {
        bool mPlaced;
        Vector2i mPosition;  // The icon glyph's center.
    };

    // The glyphs, in storage the caller provides, and the text's extent.
    struct Result {
        RndFontStyleSize mStyleSize;  // The style size that fit.
        BufVector<Glyph> mGlyphs;
        BufVector<PageGlyphs> mPages;  // Left untouched without a capacity.
        BufVector<IconResult> mIcons;  // One per params icon.
        Vector2i mMin;
        Vector2i mMax;
        Vector2i mEnd;  // Where the next glyph would go.
    };

    // The characters IsNumeric accepts.
    static const char* GetNumericChars();  // 0x67D910
    // The glyphs the text has, skipping markup tags when `markup` is set.
    // Whitespace does not count.
    static unsigned long CalcNumGlyphs(const unsigned short* text, bool markup);  // 0x67D920
    // The glyphs a result needs: three more for the truncation periods.
    static unsigned long CalcResultGlyphsCapacity(
        unsigned long numGlyphs,
        unsigned long numIcons,
        RndTextFitMode fitMode);  // 0x67E570
    static void ProcessText(const Params& params, Result& result);  // 0x67E590
    // Whether every glyph of the text is a numeric character.
    static bool IsNumeric(const unsigned short* text, bool markup);  // 0x67E100
    // Replaces each non-numeric glyph with a block (U+25A0).
    static void EnforceNumeric(unsigned short* text, bool markup);  // 0x67E350
    // Ends the text after its first `numGlyphs` glyphs.
    static void TruncateNumGlyphs(unsigned short* text, bool markup, unsigned long numGlyphs);  // 0x67DD00

private:
    // A line's glyphs, [mStart, mEnd), and its width. Name not in the
    // reference map.
    struct Line {
        unsigned long mStart;
        unsigned long mEnd;
        int mWidth;
    };

    // The layout's progress through the text.
    struct Context {
        Vector2i mResolution;  // The output resolution, which picks the font sizes.
        const unsigned short* mText;
        unsigned short mPrevChar;  // Zero after a line break or an icon.
        RndFont* mPrevFont;
        bool mTruncated;           // Stops the layout.
        FixedVector<unsigned long, 8> mStyleStack;
        FixedVector<Line, 64> mLines;
        BufVector<BufVector<PageGlyphs>> mPageGlyphs;  // Per style, per page.
    };

    // Inlined into IsNumeric and EnforceNumeric.
    static bool _IsCharNumeric(unsigned short ch);
    // Steps over one whitespace character or markup tag.
    static bool _SkipWhitespaceAndMarkup(const unsigned short*& text, bool markup);  // 0x67DB30
    static bool _SkipWhitespaceAndMarkup(unsigned short*& text, bool markup);        // 0x67DF30
    // Parses a <name=value> tag at `it` and steps past it. `text` names the
    // whole text in warnings and may be null.
    static bool _TryParseMarkup(
        const unsigned short* text,
        const unsigned short*& it,
        char* name,
        char* value);  // 0x680DA0
    static bool _TryParseMarkup(
        const Params& params,
        Context& context,
        char* name,
        char* value);  // Inlined into 0x67F8A0.

    static void _InitResults(const Params& params, Result& result, RndFontStyleSize styleSize);
    static void _InitIconResults(const Params& params, Result& result);
    static void _InitContext(const Params& params, const Result& result, Context& context);
    static void _GenGlyphs(const Params& params, Result& result, Context& context);  // 0x67EF70
    static bool _TryProcessMarkup(const Params& params, Result& result, Context& context);  // 0x67F8A0
    static bool _TryProcessWhitespace(const Params& params, Result& result, Context& context);  // 0x67FF00
    // The map has _ProcessOneGlyph(Params const&, Result&, Context&,
    // unsigned short, unsigned long, RndFontGlyph const*); this build adds
    // the glyph's page.
    static void _ProcessOneGlyph(
        const Params& params,
        Result& result,
        Context& context,
        unsigned short ch,
        unsigned long charIndex,
        unsigned long page,
        const RndFontGlyph* glyph);  // 0x680320
    static void _InsertIcon(
        const Params& params,
        Result& result,
        Context& context,
        unsigned long icon);  // 0x680B60
    // Wraps the lines at the width. Name not in the reference map.
    static void _ApplyWordWrap(const Params& params, Result& result, Context& context);  // 0x67F1B0
    // Splits one line until each part fits the width. The binary takes two
    // more, unused arguments. Name not in the reference map.
    static void _WrapLine(
        int maxWidth,
        int lineHeight,
        BufVector<Glyph>& glyphs,
        FixedVector<Line, 64>& lines,
        Line line);  // 0x681470
    static void _ApplyAlignment(const Params& params, Result& result, Context& context);
    static void _ApplyJustification(const Params& params, Result& result, Context& context);  // 0x67F560
    static void _FinalizeResult(const Params& params, Result& result, const Context& context);  // 0x67F640
    static void _FinalizeIconResults(const Params& params, Result& result);
    // The prefix of the layout's error messages. Name not in the reference
    // map.
    static const char* _MakeErrorPrefix(const Params& params);  // 0x680210
};

static_assert(sizeof(RndTypesetter::StyleFont) == 16);
static_assert(offsetof(RndTypesetter::Style, mAlignment) == 8);
static_assert(offsetof(RndTypesetter::Style, mFonts) == 16);
static_assert(sizeof(RndTypesetter::Style) == 120);
static_assert(offsetof(RndTypesetter::Icon, mWidth) == 8);
static_assert(offsetof(RndTypesetter::Icon, mVerticalOffset) == 12);
static_assert(sizeof(RndTypesetter::Icon) == 16);
static_assert(offsetof(RndTypesetter::Params, mCapitalization) == 24);
static_assert(offsetof(RndTypesetter::Params, mAlignment) == 28);
static_assert(offsetof(RndTypesetter::Params, mJustification) == 32);
static_assert(offsetof(RndTypesetter::Params, mFitMode) == 36);
static_assert(offsetof(RndTypesetter::Params, mMaxWidth) == 40);
static_assert(offsetof(RndTypesetter::Params, mStyleSize) == 48);
static_assert(offsetof(RndTypesetter::Params, mMarkup) == 52);
static_assert(offsetof(RndTypesetter::Params, mIcons) == 56);
static_assert(offsetof(RndTypesetter::Params, mObject) == 72);
static_assert(offsetof(RndTypesetter::Params, mToken) == 80);
static_assert(sizeof(RndTypesetter::Params) == 88);
static_assert(offsetof(RndTypesetter::Glyph, mUV) == 16);
static_assert(offsetof(RndTypesetter::Glyph, mStyle) == 32);
static_assert(offsetof(RndTypesetter::Glyph, mLineBottom) == 40);
static_assert(offsetof(RndTypesetter::Glyph, mCharIndex) == 48);
static_assert(offsetof(RndTypesetter::Glyph, mBreakBefore) == 52);
static_assert(offsetof(RndTypesetter::Glyph, mBearing) == 56);
static_assert(sizeof(RndTypesetter::Glyph) == 64);
static_assert(sizeof(RndTypesetter::PageGlyphs) == 24);
static_assert(offsetof(RndTypesetter::IconResult, mPosition) == 4);
static_assert(sizeof(RndTypesetter::IconResult) == 12);
static_assert(offsetof(RndTypesetter::Result, mGlyphs) == 8);
static_assert(offsetof(RndTypesetter::Result, mPages) == 32);
static_assert(offsetof(RndTypesetter::Result, mIcons) == 56);
static_assert(offsetof(RndTypesetter::Result, mMin) == 80);
static_assert(offsetof(RndTypesetter::Result, mEnd) == 96);
static_assert(sizeof(RndTypesetter::Result) == 104);
