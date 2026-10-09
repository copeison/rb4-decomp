#include "render/fonts/RndFontPage.h"

#include "render/fonts/RndFont.h"
#include "render/textures/RndTexture2D.h"

// Reconstructed from eboot.elf at 0x6671B0.
RndFontPage::RndFontPage() : mTexture(nullptr), mFont(nullptr), mResolution{0, 0} {}

// Reconstructed from eboot.elf at 0x6671F0.
RndFontPage::~RndFontPage() {
    delete mTexture;
    mTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x667240.
void RndFontPage::PostConstruct(RndFont* font, const Vector2i& resolution) {
    mFont = font;
    mResolution = resolution;
}

// Reconstructed from eboot.elf at 0x667250. A binary search of the glyphs,
// which Finalize leaves sorted by character.
const RndFontGlyph* RndFontPage::FindGlyph(unsigned short character) const {
    const RndFontGlyph* glyph = mGlyphs.begin();
    long count = mGlyphs.end() - mGlyphs.begin();
    while (count > 0) {
        const long half = count / 2;
        if (glyph[half].mChar < character) {
            glyph += half + 1;
            count -= half + 1;
        } else {
            count = half;
        }
    }
    if (glyph == mGlyphs.end() || glyph->mChar != character) {
        return nullptr;
    }
    return glyph;
}

// Reconstructed from eboot.elf at 0x667340.
void RndFontPage::SetTexture(RndTexture2D* texture) {
    mTexture = texture;
}

// Reconstructed from eboot.elf at 0x667350.
void RndFontPage::SetNumGlyphs(unsigned long count) {
    mGlyphs.resize(count);
}

// Reconstructed from eboot.elf at 0x667380. The binary looks the size up
// twice, once for the tile size and once for the glyph height.
void RndFontPage::SetGlyph(
    unsigned long index,
    unsigned short character,
    int width,
    int unknown8,
    int unknown12,
    const Vector2i& textureSize) {
    RndFontGlyph& glyph = mGlyphs[index];
    glyph.mUnknown2 = 0;
    glyph.mChar = character;
    glyph.mWidth = width;
    glyph.mUnknown8 = unknown8;
    glyph.mUnknown12 = unknown12;

    const Vector2i& tileSize = mFont->GetSize(mResolution)->mGlyphTileSize;
    const int tilesPerRow = textureSize.x / tileSize.x;
    const float invWidth = 1.0F / static_cast<float>(textureSize.x);
    const unsigned long row = index / static_cast<unsigned long>(static_cast<long>(tilesPerRow));
    const unsigned long column = index % static_cast<unsigned long>(static_cast<long>(tilesPerRow));
    const float invHeight = 1.0F / static_cast<float>(textureSize.y);
    glyph.mUV[0] = static_cast<float>(static_cast<int>(column) * tileSize.x) * invWidth;
    glyph.mUV[1] = invHeight * static_cast<float>(static_cast<int>(row) * tileSize.y);
    glyph.mUV[2] = invWidth * static_cast<float>(width);
    glyph.mUV[3] =
        static_cast<float>(mFont->GetSize(mResolution)->mGlyphHeight) * invHeight;
}

// Reconstructed from eboot.elf at 0x6674B0.
void RndFontPage::Finalize() {}
