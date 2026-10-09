#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class RndFontPage;
struct RndFontGlyph;

// A bitmap font. This build keeps one set of metrics and pages per output
// resolution; the setters take the resolution whose entry they change and
// fall back to the first entry when it has none. Only the members the debug
// font uses are declared.
class RndFont {
public:
    // A kerning table entry. The key holds the first character in its low
    // 16 bits and the second in its high 16 bits; Finalize sorts the table
    // by it, compared as an unsigned int. Field names are not in the
    // reference map.
    struct KerningPair {
        bool operator<(const KerningPair& other) const {
            return mKey < other.mKey;
        }

        unsigned int mKey;
        int mKerning;  // Pixels added between the two characters.
    };

    // The metrics and pages for one output resolution. Name not in the
    // reference map; field names are not in the reference map, and which of
    // the 16, 20 and 32 fields is the glyph height, the space size and the
    // fixed width is inferred from the setter order.
    struct Size {
        Vector2i mResolution;
        Vector2i mGlyphTileSize;
        int mGlyphHeight;
        int mSpaceSize;
        Vector2i mGlyphSpacing;
        int mGlyphFixedWidth;
        unsigned long mNumPages;
        RndFontPage* mPages;  // An array of mNumPages.
        eastl::vector<KerningPair> mKerningTable;
    };

    RndFont();   // 0x65D1F0
    // Deletes every size's pages.
    ~RndFont();  // 0x65D220

    // Reconstructed from eboot.elf at 0x65C390. Appends a copy of a zeroed
    // entry for the resolution. The binary's copy sits in the debug font's
    // object, so the original is likely inline in this header. Name not in
    // the reference map.
    void AddResolution(const Vector2i& resolution) {
        Size size{};
        size.mResolution = resolution;
        mSizes.push_back(size);
    }
    // The entry for the resolution, or the first entry. Inlined into the
    // setters and into RndDebugFont::Init at 0x65C2A8. Name not in the
    // reference map.
    Size* GetSize(const Vector2i& resolution) {
        Size* size = mSizes.begin();
        for (Size* it = mSizes.begin(); it != mSizes.end(); ++it) {
            if (it->mResolution.x == resolution.x && it->mResolution.y == resolution.y) {
                size = it;
                break;
            }
        }
        return size;
    }
    const Size* GetSize(const Vector2i& resolution) const {
        return const_cast<RndFont*>(this)->GetSize(resolution);
    }

    // The map's setters take only the value; this build adds the
    // resolution.
    void SetGlyphTileSizeInPixels(const Vector2i& resolution, const Vector2i& size);     // 0x65D320
    void SetGlyphHeightInPixels(const Vector2i& resolution, int height);                 // 0x65D360
    void SetSpaceSizeInPixels(const Vector2i& resolution, int size);                     // 0x65D3A0
    void SetGlyphSpacingInPixels(const Vector2i& resolution, const Vector2i& spacing);   // 0x65D3E0
    void SetGlyphFixedWidthInPixels(const Vector2i& resolution, int width);              // 0x65D420
    // Allocates the pages and links them to the font.
    void SetNumPages(const Vector2i& resolution, unsigned long count);                   // 0x65D460
    // Finalizes every page and sorts each kerning table.
    void Finalize();  // 0x65D910

    // The glyph of the character in the resolution's size and the page that
    // holds it, or null. The first page whose character range covers the
    // character is searched. The map has FindGlyphOnPage(unsigned short,
    // unsigned long&) const; this build adds the resolution.
    const RndFontGlyph* FindGlyphOnPage(
        const Vector2i& resolution,
        unsigned short character,
        unsigned long& page) const;  // 0x65D600
    // The kerning between the two characters in the resolution's size, or
    // zero. The map has GetKerning(unsigned short, unsigned short) const;
    // this build adds the resolution.
    int GetKerning(
        const Vector2i& resolution,
        unsigned short first,
        unsigned short second) const;  // 0x65D690

    // Field names are not in the reference map. For a font with bit 0 of
    // mFlags set, the typesetter adds the glyph spacing after each glyph
    // instead of between glyphs, and sizes spaces from the space size.
    Symbol mName;
    int mFlags;  // Name not in the reference map.
    eastl::vector<Size> mSizes;
};

static_assert(offsetof(RndFont::KerningPair, mKerning) == 4);
static_assert(sizeof(RndFont::KerningPair) == 8);
static_assert(offsetof(RndFont::Size, mGlyphTileSize) == 8);
static_assert(offsetof(RndFont::Size, mGlyphHeight) == 16);
static_assert(offsetof(RndFont::Size, mSpaceSize) == 20);
static_assert(offsetof(RndFont::Size, mGlyphSpacing) == 24);
static_assert(offsetof(RndFont::Size, mGlyphFixedWidth) == 32);
static_assert(offsetof(RndFont::Size, mNumPages) == 40);
static_assert(offsetof(RndFont::Size, mPages) == 48);
static_assert(offsetof(RndFont::Size, mKerningTable) == 56);
static_assert(sizeof(RndFont::Size) == 88);
static_assert(offsetof(RndFont, mFlags) == 8);
static_assert(offsetof(RndFont, mSizes) == 16);
static_assert(sizeof(RndFont) == 48);
