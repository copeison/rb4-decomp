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
