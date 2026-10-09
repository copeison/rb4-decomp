#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "math/vector/Vector3i.h"

// Floating-point image used as a conversion source. The vtable is at
// 0x1932C08 and holds only the destructors. Field names are not in the
// reference map.
class RndPixelCanvas {
public:
    RndPixelCanvas();           // 0x681630
    virtual ~RndPixelCanvas();  // 0x681800, 0x6818B0

    // Each dimension is at least one; the pixels are opaque black, the
    // binary's default Hmx::Color.
    void CreateUninitialized(int width, int height, int depth);  // 0x681A00
    // Each dimension is at least one.
    void CreateWithColor(const Vector3i& size, const Hmx::Color& color);  // 0x681AC0
    void CreateWithColor(
        int width,
        int height,
        int depth,
        const Hmx::Color& color);  // 0x681C00
    // Releases the pixels and the mips and clears the size. No out-of-line
    // copy is located in this build; the destructor and the Create
    // functions inline it.
    void Free();

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
