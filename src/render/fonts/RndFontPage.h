#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "render/fonts/RndFontGlyph.h"
#include "utl/containers/Vector.h"

class RndFont;
class RndTexture2D;

// One texture of a font size and the glyphs drawn from it. Only the members
// the debug font uses are declared. Field names are not in the reference
// map.
class RndFontPage {
public:
    RndFontPage();   // 0x6671B0
    // Deletes the texture.
    ~RndFontPage();  // 0x6671F0

    // Links the page to its font and the output resolution of its size. The
    // map has PostConstruct(RndFont*); this build adds the resolution.
    void PostConstruct(RndFont* font, const Vector2i& resolution);  // 0x667240
    // Takes ownership of the texture.
    void SetTexture(RndTexture2D* texture);  // 0x667340
    void SetNumGlyphs(unsigned long count);  // 0x667350
    // Records the glyph's character and metrics and derives its texture
    // rectangle from the size's glyph tiles. The map has SetGlyph(unsigned
    // long, unsigned short, int, Vector2i const&); this build stores two
    // more metrics.
    void SetGlyph(
        unsigned long index,
        unsigned short character,
        int width,
        int unknown8,
        int unknown12,
        const Vector2i& textureSize);  // 0x667380

    RndTexture2D* mTexture;
    eastl::vector<RndFontGlyph> mGlyphs;
    RndFont* mFont;
    Vector2i mResolution;
};

static_assert(offsetof(RndFontPage, mGlyphs) == 8);
static_assert(offsetof(RndFontPage, mFont) == 40);
static_assert(offsetof(RndFontPage, mResolution) == 48);
static_assert(sizeof(RndFontPage) == 56);
