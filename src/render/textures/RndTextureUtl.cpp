#include "render/textures/RndTextureUtl.h"

#include "math/color/Color.h"
#include "render/defaults/RndDefaults.h"
#include "render/textures/RndPixelCanvas.h"

// Indexed by RndDefaultTextureType. Name not in the reference map.
static const char* const sDefaultTextureNames[kNumDefaultTextureTypes] = {
    "White",
    "Black",
    "Zero",
    "Flat Normal",
    "Error",
    "Error Greyscale",
    "Error Normal",
};  // 0x1936800

// Reconstructed from eboot.elf at 0x6AC700.
const char* RndTextureUtl::GetDefaultTextureName(RndDefaultTextureType type) {
    return sDefaultTextureNames[type];
}

// Reconstructed from eboot.elf at 0x6AED00. The row loop is bounded by the
// height rather than the width, so only square slices are fully patterned;
// a 1D canvas gets just its first texel.
void RndTextureUtl::FillCheckerboard(
    RndPixelCanvas& canvas,
    const Hmx::Color& primary,
    const Hmx::Color& secondary,
    int cellSize) {
    for (int z = 0; z < canvas.mDepth; ++z) {
        for (int y = 0; y < canvas.mHeight; ++y) {
            for (int x = 0; x < canvas.mHeight; ++x) {
                const bool odd = (((z / cellSize) ^ (y / cellSize)
                                   ^ (x / cellSize)) & 1) != 0;
                canvas.mPixels[x + canvas.mWidth * (y + z * canvas.mHeight)] =
                    odd ? primary : secondary;
            }
        }
    }
}
