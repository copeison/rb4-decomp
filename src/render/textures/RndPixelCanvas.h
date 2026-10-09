#pragma once

#include <cstddef>

#include "math/color/Color.h"

// Floating-point image used as a conversion source. The vtable is at
// 0x1932C08 and holds only the destructors. Field names are not in the
// reference map.
class RndPixelCanvas {
public:
    RndPixelCanvas();           // 0x681630
    virtual ~RndPixelCanvas();  // 0x681800, 0x6818B0

    // Not reconstructed yet. Each dimension is at least one; the pixels
    // keep Hmx::Color's default value.
    void CreateUninitialized(int width, int height, int depth);  // 0x681A00
    // Not reconstructed yet. Each dimension is at least one.
    void CreateWithColor(
        int width,
        int height,
        int depth,
        const Hmx::Color& color);  // 0x681C00

    int mWidth;
    int mHeight;
    int mDepth;
    Hmx::Color* mPixels;
    // The next smaller mip level.
    RndPixelCanvas* mMip;
};

static_assert(offsetof(RndPixelCanvas, mWidth) == 8);
static_assert(offsetof(RndPixelCanvas, mDepth) == 16);
static_assert(offsetof(RndPixelCanvas, mPixels) == 24);
static_assert(offsetof(RndPixelCanvas, mMip) == 32);
static_assert(sizeof(RndPixelCanvas) == 40);
