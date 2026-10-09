#include "render/fonts/RndTypesetter.h"

#include <cstring>

#include "entity/core/GameObject.h"
#include "render/debug/RndDebugFont.h"
#include "render/fonts/RndFont.h"
#include "render/fonts/RndFontGlyph.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "utl/containers/Vector.h"
#include "utl/text/MakeString.h"
#include "utl/text/UTF8.h"

namespace {

// Set by the static initializer at 0x681610 and never read; the same triple
// follows other render objects. Names not in the reference map.
[[maybe_unused]] int gTypesetterUnknown = -1;  // 0x1AAE28C
[[maybe_unused]] int gTypesetterUnknown2 = 8;  // 0x1AAE290
[[maybe_unused]] int gTypesetterUnknown3 = 4;  // 0x1AAE294

// The block glyph drawn for a missing or rejected character. Name not in
// the reference map.
constexpr unsigned short kBlockChar = 0x25A0;
// A markup tag's name and value keep 64 characters. Name not in the
// reference map.
constexpr unsigned long kMaxMarkupLength = 64;

// Tab, line feed, carriage return and space. Name not in the reference map.
bool IsBreakingSpace(unsigned short ch) {
    return ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ';
}

// The breaking spaces and the no-break space. Name not in the reference map.
bool IsSpace(unsigned short ch) {
    return IsBreakingSpace(ch) || ch == 0xA0;
}

// The release build drops the layout's warnings; only the conversion of the
// text for the message remains. Name not in the reference map.
void WarnAboutText(const unsigned short* text) {
    char buffer[512] = {};
    WideCharToChar(text, buffer, sizeof(buffer));
}

// Reconstructed from eboot.elf at 0x6811F0. Collects the indices of the
// characters a line may break before: each space or zero-width space, each
// character after a hyphen, and a CJK boundary the kinsoku rules allow. Name
// not in the reference map.
void FindLineBreaks(const unsigned short* text, eastl::vector<unsigned int>& breaks) {
    for (unsigned int i = 0; text[i] != 0; ++i) {
        if (i == 0) {
            continue;
        }
        const unsigned short ch = text[i];
        const unsigned short prev = text[i - 1];
        if (IsBreakingSpace(ch) || ch == 0x200B) {
            breaks.push_back(i);
        } else if (
            prev == '-' ||
            ((IsCJKChar(prev) || IsCJKChar(ch)) && CanBeginLine(ch) && CanEndLine(prev))) {
            breaks.push_back(i);
        }
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x67D910.
const char* RndTypesetter::GetNumericChars() {
    return "0123456789.,-";
}

// Reconstructed from eboot.elf at 0x67D920.
unsigned long RndTypesetter::CalcNumGlyphs(const unsigned short* text, bool markup) {
    unsigned long count = 0;
    while (*text != 0) {
        if (_SkipWhitespaceAndMarkup(text, markup)) {
            continue;
        }
        ++count;
        ++text;
    }
    return count;
}

// Reconstructed from eboot.elf at 0x67DB30. The out-of-line copy has no
// callers; every user inlines it.
bool RndTypesetter::_SkipWhitespaceAndMarkup(const unsigned short*& text, bool markup) {
    if (IsSpace(*text)) {
        ++text;
        return true;
    }
    if (!markup) {
        return false;
    }
    char name[kMaxMarkupLength + 1];
    char value[kMaxMarkupLength + 1];
    return _TryParseMarkup(nullptr, text, name, value);
}

// Reconstructed from eboot.elf at 0x67DD00.
void RndTypesetter::TruncateNumGlyphs(
    unsigned short* text,
    bool markup,
    unsigned long numGlyphs) {
    unsigned long count = 0;
    while (*text != 0) {
        if (_SkipWhitespaceAndMarkup(text, markup)) {
            continue;
        }
        if (count == numGlyphs) {
            *text = 0;
            return;
        }
        ++count;
        ++text;
    }
}

// Reconstructed from eboot.elf at 0x67DF30. The out-of-line copy has no
// callers; every user inlines it.
bool RndTypesetter::_SkipWhitespaceAndMarkup(unsigned short*& text, bool markup) {
    if (IsSpace(*text)) {
        ++text;
        return true;
    }
    if (!markup) {
        return false;
    }
    char name[kMaxMarkupLength + 1];
    char value[kMaxMarkupLength + 1];
    const unsigned short* end = text;
    if (!_TryParseMarkup(nullptr, end, name, value)) {
        return false;
    }
    text += end - text;
    return true;
}

// Reconstructed from eboot.elf at 0x67E100.
bool RndTypesetter::IsNumeric(const unsigned short* text, bool markup) {
    while (*text != 0) {
        if (_SkipWhitespaceAndMarkup(text, markup)) {
            continue;
        }
        if (!_IsCharNumeric(*text)) {
            return false;
        }
        ++text;
    }
    return true;
}

// Reconstructed from the copies inlined into 0x67E100 and 0x67E350, which
// test a bit mask of the characters.
bool RndTypesetter::_IsCharNumeric(unsigned short ch) {
    for (const char* numeric = GetNumericChars(); *numeric != '\0'; ++numeric) {
        if (ch == static_cast<unsigned char>(*numeric)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x67E350.
void RndTypesetter::EnforceNumeric(unsigned short* text, bool markup) {
    while (*text != 0) {
        if (_SkipWhitespaceAndMarkup(text, markup)) {
            continue;
        }
        if (!_IsCharNumeric(*text)) {
            *text = kBlockChar;
        }
        ++text;
    }
}

// Reconstructed from eboot.elf at 0x67E570.
unsigned long RndTypesetter::CalcResultGlyphsCapacity(
    unsigned long numGlyphs,
    unsigned long numIcons,
    RndTextFitMode fitMode) {
    const unsigned long capacity = numGlyphs + numIcons;
    return fitMode == kTextFitModeTruncate ? capacity + 3 : capacity;
}

// Reconstructed from eboot.elf at 0x67E590. The page counts live on the
// stack, sized by each style's largest page count over its sizes. The
// layout runs once per style size until the text fits; when no size fits,
// it runs once more with the last size, wrapping without shrinking or not
// fitting at all.
void RndTypesetter::ProcessText(const Params& params, Result& result) {
    Params layout = params;
    const unsigned long numSizes = layout.mStyles[0].mFonts.size();

    Context context;
    context.mResolution = TheRndDevice()->mSettings->mOutputResolution;
    context.mText = nullptr;
    context.mPrevChar = 0;
    context.mPrevFont = nullptr;
    context.mTruncated = false;

    const unsigned long numStyles = layout.mNumStyles;
    BufVector<PageGlyphs>* styles = nullptr;
    if (numStyles != 0) {
        styles = static_cast<BufVector<PageGlyphs>*>(
            __builtin_alloca(numStyles * sizeof(BufVector<PageGlyphs>)));
    }
    context.mPageGlyphs = BufVector<BufVector<PageGlyphs>>(styles, numStyles);
    context.mPageGlyphs.resize(numStyles);
    for (unsigned long style = 0; style < numStyles; ++style) {
        unsigned long maxPages = 0;
        const FixedVector<StyleFont, 5>& fonts = layout.mStyles[style].mFonts;
        for (unsigned long size = 0; size < fonts.size(); ++size) {
            const unsigned long numPages = fonts[size].mFont->GetSize(context.mResolution)->mNumPages;
            if (maxPages < numPages) {
                maxPages = numPages;
            }
        }
        PageGlyphs* pages = nullptr;
        if (maxPages != 0) {
            pages = static_cast<PageGlyphs*>(__builtin_alloca(maxPages * sizeof(PageGlyphs)));
        }
        context.mPageGlyphs[style] = BufVector<PageGlyphs>(pages, maxPages);
    }

    RndFontStyleSize styleSize = layout.mStyleSize;
    for (;;) {
        _InitResults(layout, result, styleSize);
        _InitContext(layout, result, context);
        _GenGlyphs(layout, result, context);
        _ApplyWordWrap(layout, result, context);

        bool fits = true;
        if (layout.mFitMode == kTextFitModeShrinkToFit) {
            if (layout.mMaxWidth > 0) {
                for (const Line& line : context.mLines) {
                    if (line.mWidth > layout.mMaxWidth) {
                        fits = false;
                        break;
                    }
                }
            }
        } else if (layout.mFitMode != kTextFitModeWrapAndShrink) {
            break;
        }
        if (fits && layout.mMaxHeight > 0 && !result.mGlyphs.empty()) {
            fits = result.mGlyphs[0].mMax.y - result.mGlyphs.back().mMin.y <= layout.mMaxHeight;
        }
        if (fits) {
            break;
        }
        if (static_cast<unsigned long>(styleSize) + 1 < numSizes) {
            styleSize = static_cast<RndFontStyleSize>(styleSize + 1);
            continue;
        }
        WarnAboutText(layout.mText);
        layout.mFitMode = layout.mFitMode == kTextFitModeWrapAndShrink ? kTextFitModeWordWrap
                                                                      : kTextFitModeNone;
    }

    _ApplyAlignment(layout, result, context);
    _ApplyJustification(layout, result, context);
    _FinalizeResult(layout, result, context);
}

// Reconstructed from the copy inlined into 0x67E590. The pages are left as
// they are.
void RndTypesetter::_InitResults(
    const Params& params,
    Result& result,
    RndFontStyleSize styleSize) {
    result.mEnd = {0, 0};
    result.mStyleSize = styleSize;
    result.mGlyphs.mSize = 0;
    _InitIconResults(params, result);
}

// Reconstructed from the copy inlined into 0x67E590.
void RndTypesetter::_InitIconResults(const Params& params, Result& result) {
    result.mIcons.resize(params.mNumIcons);
    for (IconResult& icon : result.mIcons) {
        icon.mPlaced = false;
        icon.mPosition = {0, 0};
    }
}

// Reconstructed from the copy inlined into 0x67E590. Each style's page
// counts restart at zero for the style size's font.
void RndTypesetter::_InitContext(const Params& params, const Result& result, Context& context) {
    context.mText = params.mText;
    context.mPrevChar = 0;
    context.mPrevFont = nullptr;
    context.mTruncated = false;
    context.mLines.clear();
    context.mLines.push_back(Line{0, 0, 0});
    context.mStyleStack.clear();
    context.mStyleStack.push_back(0);
    for (unsigned long style = 0; style < context.mPageGlyphs.size(); ++style) {
        const RndFont* font = params.mStyles[style].mFonts[result.mStyleSize].mFont;
        const unsigned long numPages = font->GetSize(context.mResolution)->mNumPages;
        BufVector<PageGlyphs>& pages = context.mPageGlyphs[style];
        pages.resize(numPages);
        for (unsigned long page = 0; page < numPages; ++page) {
            pages[page] = PageGlyphs{style, page, 0};
        }
    }
}

// Reconstructed from eboot.elf at 0x67EF70. A character the style's font
// lacks comes from the style's fallback when extended fonts are loaded,
// and is drawn as a block otherwise; without a block glyph it is skipped.
void RndTypesetter::_GenGlyphs(const Params& params, Result& result, Context& context) {
    while (*context.mText != 0 && !context.mTruncated) {
        if (_TryProcessMarkup(params, result, context) ||
            _TryProcessWhitespace(params, result, context)) {
            continue;
        }
        const unsigned short* text = context.mText;
        unsigned short ch = *text;
        context.mText = text + 1;
        const unsigned long charIndex = static_cast<unsigned long>(text - params.mText);
        if (params.mCapitalization == kTextCapitalizationUpper) {
            ch = WToUpper(ch);
        } else if (params.mCapitalization == kTextCapitalizationLower) {
            ch = WToLower(ch);
        }

        const unsigned long style = context.mStyleStack.back();
        const RndFont* font = params.mStyles[style].mFonts[result.mStyleSize].mFont;
        unsigned long page;
        const RndFontGlyph* glyph = font->FindGlyphOnPage(context.mResolution, ch, page);
        if (glyph == nullptr) {
            if (RndDebugFont::HasExtendedFonts()) {
                const unsigned long fallback = style + params.mNumStyles / 2;
                const RndFont* fallbackFont =
                    params.mStyles[fallback].mFonts[result.mStyleSize].mFont;
                if (fallbackFont != nullptr) {
                    const RndFontGlyph* fallbackGlyph =
                        fallbackFont->FindGlyphOnPage(context.mResolution, ch, page);
                    if (fallbackGlyph != nullptr) {
                        context.mStyleStack.push_back(fallback);
                        _ProcessOneGlyph(params, result, context, ch, charIndex, page, fallbackGlyph);
                        context.mStyleStack.pop_back();
                        continue;
                    }
                }
            }
            glyph = font->FindGlyphOnPage(context.mResolution, kBlockChar, page);
            if (glyph == nullptr) {
                static_cast<void>(_MakeErrorPrefix(params));
                context.mPrevChar = 0;
                context.mPrevFont = nullptr;
                continue;
            }
            ch = kBlockChar;
        }
        _ProcessOneGlyph(params, result, context, ch, charIndex, page, glyph);
    }
}

// Reconstructed from eboot.elf at 0x67F1B0. Only word wrap and wrap and
// shrink wrap, and only within a positive width. Glyphs after a break
// opportunity are marked, then every line is split.
void RndTypesetter::_ApplyWordWrap(const Params& params, Result& result, Context& context) {
    if ((params.mFitMode != kTextFitModeWrapAndShrink &&
         params.mFitMode != kTextFitModeWordWrap) ||
        params.mMaxWidth <= 0) {
        return;
    }

    eastl::vector<unsigned int> breaks;
    FindLineBreaks(params.mText, breaks);
    unsigned long next = 0;
    for (unsigned long i = 0; i < result.mGlyphs.size() && next != breaks.size(); ++i) {
        Glyph& glyph = result.mGlyphs[i];
        while (next != breaks.size() && glyph.mCharIndex != 0xFFFFFFFFU &&
               glyph.mCharIndex >= breaks[next]) {
            ++next;
            glyph.mBreakBefore = true;
        }
    }

    const RndFont* font = params.mStyles[0].mFonts[result.mStyleSize].mFont;
    const RndFont::Size* size = font->GetSize(context.mResolution);
    const int lineHeight = size->mGlyphHeight + size->mGlyphSpacing.y;
    FixedVector<Line, 64> lines;
    for (const Line& line : context.mLines) {
        _WrapLine(params.mMaxWidth, lineHeight, result.mGlyphs, lines, line);
    }
    context.mLines = lines;
}

// Reconstructed from the copy inlined into 0x67E590. Moves the text so the
// alignment's edge of the lines sits at the origin.
void RndTypesetter::_ApplyAlignment(const Params& params, Result& result, Context& context) {
    static_cast<void>(context);
    if (result.mGlyphs.empty() || params.mAlignment == kTextAlignTop) {
        return;
    }
    int offset = result.mGlyphs[0].mLineTop - result.mGlyphs.back().mLineBottom;
    if (params.mAlignment == kTextAlignMiddle) {
        offset /= 2;
    }
    for (Glyph& glyph : result.mGlyphs) {
        glyph.mMin.y += offset;
        glyph.mMax.y += offset;
        glyph.mLineBottom += offset;
        glyph.mLineTop += offset;
    }
}

// Reconstructed from eboot.elf at 0x67F560, which has no callers; ProcessText
// inlines it. Moves each line so its right edge, or its center, sits at the
// origin. Right-justified text ends at minus the last line's width.
void RndTypesetter::_ApplyJustification(
    const Params& params,
    Result& result,
    Context& context) {
    if (params.mJustification == kTextJustifyLeft) {
        return;
    }
    const unsigned long numLines = context.mLines.size();
    for (unsigned long i = 0; i < numLines; ++i) {
        const unsigned long start = context.mLines[i].mStart;
        const unsigned long end =
            i + 1 < numLines ? context.mLines[i + 1].mStart : result.mGlyphs.size();
        if (end <= start) {
            continue;
        }
        const Glyph& last = result.mGlyphs[end - 1];
        int offset = last.mBearing - (last.mMin.x + last.mExtent);
        if (params.mJustification == kTextJustifyCenter) {
            offset /= 2;
        }
        for (unsigned long glyph = start; glyph < end; ++glyph) {
            result.mGlyphs[glyph].mMin.x += offset;
            result.mGlyphs[glyph].mMax.x += offset;
        }
    }
    if (numLines != 0 && params.mJustification == kTextJustifyRight) {
        result.mEnd.x = -context.mLines.back().mWidth;
    }
}

// Reconstructed from eboot.elf at 0x67F640. The bounds cover every glyph,
// and the pages that drew glyphs are listed when the result has room for
// them.
void RndTypesetter::_FinalizeResult(
    const Params& params,
    Result& result,
    const Context& context) {
    _FinalizeIconResults(params, result);

    if (!result.mGlyphs.empty()) {
        result.mMin = result.mGlyphs[0].mMin;
        result.mMax = result.mGlyphs[0].mMax;
        for (unsigned long i = 1; i < result.mGlyphs.size(); ++i) {
            const Glyph& glyph = result.mGlyphs[i];
            if (glyph.mMin.x <= result.mMin.x) {
                result.mMin.x = glyph.mMin.x;
            }
            if (glyph.mMin.y <= result.mMin.y) {
                result.mMin.y = glyph.mMin.y;
            }
            if (result.mMax.x < glyph.mMax.x) {
                result.mMax.x = glyph.mMax.x;
            }
            if (result.mMax.y < glyph.mMax.y) {
                result.mMax.y = glyph.mMax.y;
            }
        }
    }

    if (result.mPages.mCapacity == 0) {
        return;
    }
    for (unsigned long style = 0; style < params.mNumStyles; ++style) {
        const RndFont* font = params.mStyles[style].mFonts[result.mStyleSize].mFont;
        const unsigned long numPages = font->GetSize(context.mResolution)->mNumPages;
        for (unsigned long page = 0; page < numPages; ++page) {
            const PageGlyphs& pageGlyphs = context.mPageGlyphs[style][page];
            if (pageGlyphs.mNumGlyphs != 0) {
                result.mPages.push_back(pageGlyphs);
            }
        }
    }
}

// Reconstructed from the copy inlined into 0x67F640. A placed icon moves to
// its glyph's center, raised by the icon's offset.
void RndTypesetter::_FinalizeIconResults(const Params& params, Result& result) {
    for (unsigned long i = 0; i < result.mIcons.size(); ++i) {
        IconResult& icon = result.mIcons[i];
        if (!icon.mPlaced) {
            continue;
        }
        const Glyph& glyph = result.mGlyphs[icon.mPosition.x];
        icon.mPosition = {(glyph.mMin.x + glyph.mMax.x) / 2, (glyph.mMin.y + glyph.mMax.y) / 2};
        icon.mPosition.y += params.mIcons[i].mVerticalOffset;
    }
}

// Reconstructed from eboot.elf at 0x67F8A0. Handles <style=name>, which
// pushes a style, </style>, which pops one, and <icon=name>. Any parsed tag
// is consumed: unknown tags and names, and a pop of the last style, are
// ignored after a warning.
bool RndTypesetter::_TryProcessMarkup(const Params& params, Result& result, Context& context) {
    if (!params.mMarkup) {
        return false;
    }
    char name[kMaxMarkupLength + 1];
    char value[kMaxMarkupLength + 1];
    if (!_TryParseMarkup(params, context, name, value)) {
        return false;
    }
    if (name[0] == '\0') {
        return true;
    }

    if (std::strcmp(name, "style") == 0) {
        if (value[0] != '\0') {
            for (unsigned long style = 0; style < params.mNumStyles; ++style) {
                if (std::strcmp(params.mStyles[style].mName.Str(), value) == 0) {
                    context.mStyleStack.push_back(style);
                    return true;
                }
            }
        }
        WarnAboutText(params.mText);
        return true;
    }
    if (std::strcmp(name, "/style") == 0) {
        if (context.mStyleStack.size() < 2) {
            WarnAboutText(params.mText);
        } else {
            context.mStyleStack.pop_back();
        }
        return true;
    }
    if (std::strcmp(name, "icon") == 0) {
        if (value[0] != '\0') {
            for (unsigned long icon = 0; icon < params.mNumIcons; ++icon) {
                if (std::strcmp(params.mIcons[icon].mName.Str(), value) == 0) {
                    _InsertIcon(params, result, context, icon);
                    return true;
                }
            }
        }
        WarnAboutText(params.mText);
        return true;
    }
    WarnAboutText(params.mText);
    return true;
}

// Reconstructed from the copy inlined into 0x67F8A0.
bool RndTypesetter::_TryParseMarkup(
    const Params& params,
    Context& context,
    char* name,
    char* value) {
    return _TryParseMarkup(params.mText, context.mText, name, value);
}

// Reconstructed from eboot.elf at 0x67FF00. A space advances by the font's
// space width and a tab by three; a line break starts a new line below,
// spaced by the first style's font.
bool RndTypesetter::_TryProcessWhitespace(
    const Params& params,
    Result& result,
    Context& context) {
    const unsigned short ch = *context.mText;
    if (ch == '\n' || ch == '\r') {
        result.mEnd.x = 0;
        const RndFont* font = params.mStyles[0].mFonts[result.mStyleSize].mFont;
        const RndFont::Size* size = font->GetSize(context.mResolution);
        result.mEnd.y -= size->mGlyphSpacing.y + size->mGlyphHeight;
        context.mPrevChar = 0;
        context.mPrevFont = nullptr;
        const unsigned long numGlyphs = result.mGlyphs.size();
        context.mLines.push_back(Line{numGlyphs, numGlyphs, 0});
        ++context.mText;
        return true;
    }
    if (ch != '\t' && ch != ' ' && ch != 0xA0) {
        return false;
    }

    RndFont* font =
        params.mStyles[context.mStyleStack.back()].mFonts[result.mStyleSize].mFont;
    const RndFont::Size* size = font->GetSize(context.mResolution);
    int width;
    if ((font->mFlags & 1) != 0) {
        width = size->mGlyphSpacing.x + size->mSpaceSize;
    } else if (size->mGlyphFixedWidth > 0) {
        width = size->mGlyphSpacing.x + size->mGlyphFixedWidth;
    } else {
        width = size->mGlyphTileSize.x;
    }
    result.mEnd.x += ch == '\t' ? width * 3 : width;
    if (context.mPrevFont == font) {
        result.mEnd.x += font->GetKerning(context.mResolution, context.mPrevChar, ' ');
    }
    if (result.mMax.x < result.mEnd.x) {
        result.mMax.x = result.mEnd.x;
    }
    context.mPrevChar = ' ';
    context.mPrevFont = font;
    context.mLines.back().mWidth = result.mEnd.x;
    ++context.mText;
    return true;
}

// Reconstructed from eboot.elf at 0x680210. Names the object and the token
// the text came from, when it has them.
const char* RndTypesetter::_MakeErrorPrefix(const Params& params) {
    if (params.mObject != nullptr) {
        const char* object = params.mObject->MakeErrorName();
        if (params.mToken == Symbol()) {
            FormatString format("%s: ");
            format << object;
            return format.Str();
        }
        FormatString format("%s (token %s): ");
        format << object << params.mToken;
        return format.Str();
    }
    if (params.mToken == Symbol()) {
        return "";
    }
    FormatString format("token %s: ");
    format << params.mToken;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x680320. The glyph follows the previous
// one by the larger of the two fonts' spacings plus their kerning when the
// font is the same. A style past the first sits in the line by its
// alignment and offset. When truncating text that grows past the width,
// glyphs on the line are dropped until three periods fit, and the periods
// end the text.
void RndTypesetter::_ProcessOneGlyph(
    const Params& params,
    Result& result,
    Context& context,
    unsigned short ch,
    unsigned long charIndex,
    unsigned long page,
    const RndFontGlyph* glyph) {
    const unsigned long style = context.mStyleStack.back();
    const Style& styleDesc = params.mStyles[style];
    RndFont* font = styleDesc.mFonts[result.mStyleSize].mFont;
    const RndFont::Size* size = font->GetSize(context.mResolution);
    ++context.mPageGlyphs[style][page].mNumGlyphs;

    if (context.mPrevChar != 0) {
        const RndFont* prevFont = context.mPrevFont;
        int prevSpacing = 0;
        if (prevFont != nullptr && (prevFont->mFlags & 1) == 0) {
            prevSpacing = prevFont->GetSize(context.mResolution)->mGlyphSpacing.x;
        }
        int spacing = 0;
        if ((font->mFlags & 1) == 0) {
            spacing = size->mGlyphSpacing.x;
        }
        result.mEnd.x += prevSpacing < spacing ? spacing : prevSpacing;
        if (prevFont == font) {
            result.mEnd.x += font->GetKerning(context.mResolution, context.mPrevChar, ch);
        }
    }

    const RndFont* baseFont = params.mStyles[0].mFonts[result.mStyleSize].mFont;
    const RndFont::Size* baseSize = baseFont->GetSize(context.mResolution);
    const int lineTop = result.mEnd.y;
    const int lineBottom = lineTop - baseSize->mGlyphHeight;
    int top = lineTop;
    if (style != 0) {
        int alignedTop = lineTop;
        if (styleDesc.mAlignment == kTextAlignBottom) {
            alignedTop = lineBottom + size->mGlyphHeight;
        } else if (styleDesc.mAlignment == kTextAlignMiddle) {
            alignedTop = (lineTop + lineBottom) / 2 + size->mGlyphHeight / 2;
        }
        top = styleDesc.mFonts[result.mStyleSize].mVerticalOffset + alignedTop;
    }
    if (result.mEnd.x == 0) {
        result.mEnd.x = glyph->mBearing;
    }

    Glyph laidOut;
    laidOut.mMin = {result.mEnd.x, top - size->mGlyphHeight};
    laidOut.mMax = {result.mEnd.x + glyph->mWidth, top};
    for (int i = 0; i < 4; ++i) {
        laidOut.mUV[i] = glyph->mUV[i];
    }
    laidOut.mStyle = static_cast<unsigned int>(style);
    laidOut.mPage = static_cast<unsigned int>(page);
    laidOut.mLineBottom = lineBottom;
    laidOut.mLineTop = lineTop;
    laidOut.mCharIndex = static_cast<unsigned int>(charIndex);
    laidOut.mBreakBefore = false;
    laidOut.mBearing = glyph->mBearing;
    laidOut.mExtent = glyph->mExtent;
    result.mGlyphs.push_back(laidOut);
    result.mEnd.x += glyph->mWidth;
    if ((font->mFlags & 1) != 0) {
        result.mEnd.x += size->mGlyphSpacing.x;
    }
    Line& line = context.mLines.back();
    line.mWidth = result.mEnd.x;
    line.mEnd = result.mGlyphs.size();
    context.mPrevChar = ch;
    context.mPrevFont = font;

    if (result.mEnd.x > params.mMaxWidth) {
        if (params.mFitMode == kTextFitModeShrinkToFit) {
            return;
        }
        if (params.mFitMode == kTextFitModeTruncate) {
            context.mTruncated = true;
            unsigned long periodPage;
            const RndFontGlyph* period =
                baseFont->FindGlyphOnPage(context.mResolution, '.', periodPage);
            if (period != nullptr) {
                const int periodsWidth = (period->mWidth + baseSize->mGlyphSpacing.x) * 3;
                while (result.mEnd.x + periodsWidth > params.mMaxWidth) {
                    const unsigned long numGlyphs = result.mGlyphs.size();
                    if (numGlyphs <= 1 || result.mGlyphs[numGlyphs - 2].mMax.y != result.mEnd.y) {
                        break;
                    }
                    const Glyph& last = result.mGlyphs[numGlyphs - 1];
                    --context.mPageGlyphs[last.mStyle][last.mPage].mNumGlyphs;
                    result.mEnd.x = result.mGlyphs[numGlyphs - 2].mMax.x;
                    result.mGlyphs.mSize = numGlyphs - 1;
                    --line.mEnd;
                }
                for (int i = 0; i < 3; ++i) {
                    result.mEnd.x += baseSize->mGlyphSpacing.x;
                    const int y = result.mEnd.y;
                    Glyph dot;
                    dot.mMin = {result.mEnd.x, y - baseSize->mGlyphHeight};
                    dot.mMax = {result.mEnd.x + period->mWidth, y};
                    for (int j = 0; j < 4; ++j) {
                        dot.mUV[j] = period->mUV[j];
                    }
                    dot.mStyle = 0;
                    dot.mPage = static_cast<unsigned int>(periodPage);
                    dot.mLineBottom = y - baseSize->mGlyphHeight;
                    dot.mLineTop = y;
                    dot.mCharIndex = 0xFFFFFFFFU;
                    dot.mBreakBefore = false;
                    dot.mBearing = period->mBearing;
                    dot.mExtent = period->mExtent;
                    result.mGlyphs.push_back(dot);
                    result.mEnd.x += period->mWidth;
                    line.mWidth = result.mEnd.x;
                    line.mEnd = result.mGlyphs.size();
                }
                context.mPageGlyphs[0][periodPage].mNumGlyphs += 3;
            }
        }
    }

    if (baseSize->mGlyphHeight - result.mEnd.y <= params.mMaxHeight ||
        params.mFitMode != kTextFitModeWrapAndShrink) {
        if (result.mMax.x < result.mEnd.x) {
            result.mMax.x = result.mEnd.x;
        }
    }
}

// Reconstructed from eboot.elf at 0x680B60. Each icon is placed once; a
// repeat is ignored after a warning. The icon is a blank glyph of its width
// on the line.
void RndTypesetter::_InsertIcon(
    const Params& params,
    Result& result,
    Context& context,
    unsigned long icon) {
    IconResult& iconResult = result.mIcons[icon];
    if (iconResult.mPlaced) {
        WarnAboutText(params.mText);
        return;
    }
    iconResult.mPlaced = true;

    const int x = result.mEnd.x;
    const int y = result.mEnd.y;
    const int width = params.mIcons[icon].mWidth;
    const int right = x + width;
    const RndFont* font = params.mStyles[0].mFonts[result.mStyleSize].mFont;
    const int bottom = y - font->GetSize(context.mResolution)->mGlyphHeight;
    const unsigned long index = result.mGlyphs.size();

    Glyph glyph;
    glyph.mMin = {x, bottom};
    glyph.mMax = {right, y};
    for (int i = 0; i < 4; ++i) {
        glyph.mUV[i] = 0.0F;
    }
    glyph.mStyle = 0xFFFFFFFFU;
    glyph.mPage = 0xFFFFFFFFU;
    glyph.mLineBottom = bottom;
    glyph.mLineTop = y;
    glyph.mCharIndex = static_cast<unsigned int>(context.mText - params.mText) - 1;
    glyph.mBreakBefore = false;
    glyph.mBearing = 0;
    glyph.mExtent = width;
    result.mGlyphs.push_back(glyph);

    Line& line = context.mLines.back();
    line.mWidth = right;
    line.mEnd = result.mGlyphs.size();
    result.mEnd.x = right;
    context.mPrevChar = 0;
    context.mPrevFont = nullptr;
    if (result.mMax.x < right) {
        result.mMax.x = right;
    }
    iconResult.mPosition.x = static_cast<int>(index);
}

// Reconstructed from eboot.elf at 0x680DA0. A tag runs from '<' to the next
// '>' with no NUL, tab or space between; it splits at the first '=' into the
// name and the value, each cut at kMaxMarkupLength characters. A second '='
// or a character past ASCII empties both, with a warning when the text is
// given. Anything else is not a tag and leaves `it` alone.
bool RndTypesetter::_TryParseMarkup(
    const unsigned short* text,
    const unsigned short*& it,
    char* name,
    char* value) {
    if (*it != '<') {
        return false;
    }
    const unsigned short* close = it + 1;
    for (;; ++close) {
        const unsigned short ch = *close;
        if (ch == 0 || ch == '\t' || ch == ' ') {
            return false;
        }
        if (ch == '>') {
            break;
        }
    }

    bool inName = true;
    bool invalid = false;
    char* nameEnd = name;
    char* valueEnd = value;
    for (const unsigned short* c = it + 1; c < close; ++c) {
        const unsigned short ch = *c;
        if (ch == '=') {
            if (!invalid && !inName) {
                if (text != nullptr) {
                    WarnAboutText(text);
                }
                invalid = true;
            }
            inName = false;
            continue;
        }
        if (!invalid && ch > 0x7F) {
            if (text != nullptr) {
                WarnAboutText(text);
            }
            invalid = true;
        }
        if (inName) {
            if (static_cast<unsigned long>(nameEnd - name) < kMaxMarkupLength) {
                *nameEnd++ = static_cast<char>(ch);
            }
        } else if (static_cast<unsigned long>(valueEnd - value) < kMaxMarkupLength) {
            *valueEnd++ = static_cast<char>(ch);
        }
    }
    if (invalid) {
        nameEnd = name;
        valueEnd = value;
    }
    *nameEnd = '\0';
    *valueEnd = '\0';
    it = close + 1;
    return true;
}

// Reconstructed from eboot.elf at 0x681470. While the line's last glyph
// ends past the width, the line breaks before the last marked glyph at or
// before the first glyph past the width, or at that glyph. The rest of the
// line moves to the line's start, and every later glyph moves down a line.
void RndTypesetter::_WrapLine(
    int maxWidth,
    int lineHeight,
    BufVector<Glyph>& glyphs,
    FixedVector<Line, 64>& lines,
    Line line) {
    while (line.mEnd - line.mStart > 1) {
        if (glyphs[line.mEnd - 1].mMax.x <= maxWidth) {
            break;
        }
        unsigned long over = line.mStart + 1;
        while (over < line.mEnd && glyphs[over].mMax.x <= maxWidth) {
            ++over;
        }
        for (unsigned long i = over; i > line.mStart; --i) {
            if (glyphs[i].mBreakBefore) {
                over = i;
                break;
            }
        }

        const int shift = glyphs[over].mMin.x - glyphs[over].mBearing;
        for (unsigned long i = over; i < glyphs.size(); ++i) {
            Glyph& glyph = glyphs[i];
            if (i < line.mEnd) {
                glyph.mMin.x -= shift;
                glyph.mMax.x -= shift;
            }
            glyph.mMin.y -= lineHeight;
            glyph.mMax.y -= lineHeight;
            glyph.mLineBottom -= lineHeight;
            glyph.mLineTop -= lineHeight;
        }
        lines.push_back(Line{line.mStart, over, glyphs[over - 1].mMax.x});
        line.mStart = over;
        line.mWidth -= shift;
    }
    lines.push_back(line);
}
