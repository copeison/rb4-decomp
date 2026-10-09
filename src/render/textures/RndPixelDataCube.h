#pragma once

#include "render/textures/RndPixelData.h"

// The six faces of a cube texture, each with its own mip chain.
class RndPixelDataCube {
public:
    static constexpr int kNumFaces = 6;  // Name not in the reference map.

    RndPixelDataCube() = default;
    // Reconstructed from eboot.elf at 0x68CC00. Name not in the reference
    // map, which has only a plain copy constructor.
    RndPixelDataCube(const RndPixelDataCube& other, bool keepPixels = true);

    // Reconstructed from eboot.elf at 0x68D2C0 and 0x68D320. Every face must
    // be square, single-depth, and match the first face's size, format, and
    // mip count.
    bool IsValid() const;
    // Reconstructed from eboot.elf at 0x68D740.
    unsigned long GetTotalBytes() const;
    // Reconstructed from eboot.elf at 0x68D010.
    void FreeBuffers();
    // Creates every face `size` square with uninitialized pixels. The
    // map's format parameter is an RndPixelFormat. Not reconstructed.
    void CreateUninitialized(int size, int format);  // 0x68CE20
    // Records every face's size and format without pixels. Not
    // reconstructed.
    void CreateEmpty(int size, int format);  // 0x68CF50
    // Builds each face's mip chain. Not reconstructed.
    void CreateMips();  // 0x68D0B0
    // Copies each face's pixels into the existing buffers
    // (RndPixelData::TryCopyFrom), stopping at the first face that does
    // not match. Not reconstructed.
    void CopyFrom(const RndPixelDataCube& other);  // 0x68D100

    RndPixelData mFaces[kNumFaces];  // Name not in the reference map.
};

static_assert(sizeof(RndPixelDataCube) == 480);
