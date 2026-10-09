#include "render/fonts/RndFont.h"

#include <algorithm>

#include "render/fonts/RndFontPage.h"

// Reconstructed from eboot.elf at 0x65D1F0.
RndFont::RndFont() : mUnknown8(0) {}

// Reconstructed from eboot.elf at 0x65D220.
RndFont::~RndFont() {
    for (Size* size = mSizes.begin(); size != mSizes.end(); ++size) {
        delete[] size->mPages;
    }
}

// Reconstructed from eboot.elf at 0x65D320.
void RndFont::SetGlyphTileSizeInPixels(const Vector2i& resolution, const Vector2i& size) {
    GetSize(resolution)->mGlyphTileSize = size;
}

// Reconstructed from eboot.elf at 0x65D360.
void RndFont::SetGlyphHeightInPixels(const Vector2i& resolution, int height) {
    GetSize(resolution)->mGlyphHeight = height;
}

// Reconstructed from eboot.elf at 0x65D3A0.
void RndFont::SetSpaceSizeInPixels(const Vector2i& resolution, int size) {
    GetSize(resolution)->mSpaceSize = size;
}

// Reconstructed from eboot.elf at 0x65D3E0.
void RndFont::SetGlyphSpacingInPixels(const Vector2i& resolution, const Vector2i& spacing) {
    GetSize(resolution)->mGlyphSpacing = spacing;
}

// Reconstructed from eboot.elf at 0x65D420.
void RndFont::SetGlyphFixedWidthInPixels(const Vector2i& resolution, int width) {
    GetSize(resolution)->mGlyphFixedWidth = width;
}

// Reconstructed from eboot.elf at 0x65D460. The previous pages are not freed.
void RndFont::SetNumPages(const Vector2i& resolution, unsigned long count) {
    Size* size = GetSize(resolution);
    size->mNumPages = count;
    size->mPages = new RndFontPage[count];
    for (unsigned long page = 0; page < count; ++page) {
        size->mPages[page].PostConstruct(this, size->mResolution);
    }
}

// Reconstructed from eboot.elf at 0x65D600. A page whose range covers the
// character but lacks it ends the search.
const RndFontGlyph* RndFont::FindGlyphOnPage(
    const Vector2i& resolution,
    unsigned short character,
    unsigned long& page) const {
    const Size* size = GetSize(resolution);
    for (unsigned long i = 0; i < size->mNumPages; ++i) {
        const RndFontPage& candidate = size->mPages[i];
        if (candidate.mGlyphs.front().mChar <= character &&
            candidate.mGlyphs.back().mChar >= character) {
            const RndFontGlyph* glyph = candidate.FindGlyph(character);
            if (glyph == nullptr) {
                return nullptr;
            }
            page = i;
            return glyph;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x65D690. A binary search of the sorted
// kerning table.
int RndFont::GetKerning(
    const Vector2i& resolution,
    unsigned short first,
    unsigned short second) const {
    const Size* size = GetSize(resolution);
    const unsigned int key = (static_cast<unsigned int>(second) << 16) | first;
    const KerningPair* pair = std::lower_bound(
        size->mKerningTable.begin(),
        size->mKerningTable.end(),
        key,
        [](const KerningPair& entry, unsigned int value) { return entry.mKey < value; });
    if (pair != size->mKerningTable.end() && pair->mKey == key) {
        return pair->mKerning;
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x65D910. The binary sorts with EASTL's
// sort (0x65E5D0), an introsort finished by insertion sort; std::sort gives
// the same order for distinct keys.
void RndFont::Finalize() {
    for (Size* size = mSizes.begin(); size != mSizes.end(); ++size) {
        for (unsigned long page = 0; page < size->mNumPages; ++page) {
            size->mPages[page].Finalize();
        }
        std::sort(size->mKerningTable.begin(), size->mKerningTable.end());
    }
}
