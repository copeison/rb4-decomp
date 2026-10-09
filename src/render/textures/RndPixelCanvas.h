#pragma once

#include <cstddef>

#include "math/color/Color.h"

// Floating-point image used as a conversion source. Only the members read
// by RndPixelData::ConvertFrom are modeled; field names are not in the
// reference map.
class RndPixelCanvas {
public:
    void* mUnknown0;
    int mWidth;
    int mHeight;
    int mDepth;
    int mUnknown20;
    const Hmx::Color* mPixels;
    void* mUnknown32;
};

static_assert(offsetof(RndPixelCanvas, mPixels) == 24);
static_assert(sizeof(RndPixelCanvas) == 40);
