#include "render/textures/RndPixelCanvas.h"

namespace {

// The value the binary's Hmx::Color default constructor gives each new
// pixel. Name not in the reference map.
const Hmx::Color kDefaultPixel(0.0F, 0.0F, 0.0F, 1.0F);

// Allocates the pixels, set to the default color. Name not in the reference
// map.
Hmx::Color* NewPixels(int count) {
    auto* pixels = new Hmx::Color[count];
    for (int i = 0; i < count; ++i) {
        pixels[i] = kDefaultPixel;
    }
    return pixels;
}

}  // namespace

// Reconstructed from eboot.elf at 0x681630.
RndPixelCanvas::RndPixelCanvas()
    : mWidth(0), mHeight(0), mDepth(0), mPixels(nullptr), mMip(nullptr) {}

// Reconstructed from eboot.elf at 0x681800; the deleting destructor is at
// 0x6818B0.
RndPixelCanvas::~RndPixelCanvas() {
    Free();
}

void RndPixelCanvas::Free() {
    delete[] mPixels;
    mPixels = nullptr;
    delete mMip;
    mMip = nullptr;
    mDepth = 0;
    mWidth = 0;
    mHeight = 0;
}

// Reconstructed from eboot.elf at 0x681A00.
void RndPixelCanvas::CreateUninitialized(int width, int height, int depth) {
    Free();
    mWidth = width > 0 ? width : 1;
    mHeight = height > 0 ? height : 1;
    mDepth = depth > 0 ? depth : 1;
    mPixels = NewPixels(mWidth * mHeight * mDepth);
}

// Reconstructed from eboot.elf at 0x681AC0.
void RndPixelCanvas::CreateWithColor(const Vector3i& size, const Hmx::Color& color) {
    Free();
    mWidth = size.x > 0 ? size.x : 1;
    mHeight = size.y > 0 ? size.y : 1;
    mDepth = size.z > 0 ? size.z : 1;
    const int count = mWidth * mHeight * mDepth;
    mPixels = NewPixels(count);
    for (int i = 0; i < count; ++i) {
        mPixels[i] = color;
    }
}

// Reconstructed from eboot.elf at 0x681C00.
void RndPixelCanvas::CreateWithColor(
    int width,
    int height,
    int depth,
    const Hmx::Color& color) {
    CreateWithColor(Vector3i{width, height, depth}, color);
}
