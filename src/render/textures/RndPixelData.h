#pragma once

#include <cstddef>

#include "math/vector/Vector3i.h"
#include "os/memory/PoolAlloc.h"

class BinStream;
class RndPixelCanvas;

// One mip level of texture pixels, linked to the next smaller level. The
// vtable is at 0x1932C28.
class RndPixelData {
public:
    RndPixelData();  // 0x682930
    // Reconstructed from eboot.elf at 0x682960. Pixel buffers are copied
    // under the temporary heap unless `keepPixels` is set. Name not in the
    // reference map, which has only a plain copy constructor.
    RndPixelData(const RndPixelData& other, bool keepPixels = true);
    virtual ~RndPixelData();  // 0x682BC0, 0x682CB0

    // Reconstructed from eboot.elf at 0x6829A0. The map has
    // CopyFrom(RndPixelData const&).
    void CopyFrom(const RndPixelData& other, bool keepPixels);
    // Tail-calls TryCopyFrom, so the binary's return register carries its
    // result; the map gives no return type.
    void CopyFrom(const RndPixelData& other);  // 0x6836B0
    // Copies the pixels of this level and its mips into the existing
    // buffers when both have pixels and the sizes, buffer sizes and data
    // formats match.
    bool TryCopyFrom(const RndPixelData& other);  // 0x6836C0
    // Reconstructed from eboot.elf at 0x6830D0. Frees the level and records
    // its size and format without pixels. The map's format parameter is an
    // RndPixelFormat.
    void CreateEmpty(int width, int height, int depth, int format);
    // Reconstructed from eboot.elf at 0x682E80. The map's format parameter
    // is an RndPixelFormat.
    void Create(const Vector3i& size, int format, const void* pixels);
    // Reconstructed from eboot.elf at 0x683000. The map's format parameter
    // is an RndPixelFormat.
    void Create(int width, int height, int depth, int format, const void* pixels);
    // Reconstructed from eboot.elf at 0x684960.
    bool ConvertFrom(const RndPixelCanvas& canvas);
    // Reconstructed from eboot.elf at 0x683260.
    void FreeBuffers();
    // Reconstructed from eboot.elf at 0x686340.
    unsigned long GetTotalBytes() const;
    unsigned long GetNumMips() const;
    // Reads the pixels of this level and its mips. Revisions up to 5 derive
    // each level's size from its format; later ones store it.
    void LoadBuffers(BinStream& stream);  // 0x686B10

    POOL_OVERLOAD(RndPixelData)

    // Field names are not in the reference map.
    Vector3i mSize;
    int mFormat;
    void* mBuffer;
    unsigned long mBufferSize;
    RndPixelData* mMip;
    unsigned int mFlags;  // 4: no mips are loaded.
    unsigned int mUnknown52[5];
    void* mUnknown72;

    // Releases the pixels and mips and resets the size and format. Also the
    // destructor's body.
    void Free();  // 0x682C40
};

static_assert(offsetof(RndPixelData, mSize) == 8);
static_assert(offsetof(RndPixelData, mFormat) == 20);
static_assert(offsetof(RndPixelData, mBuffer) == 24);
static_assert(offsetof(RndPixelData, mMip) == 40);
static_assert(offsetof(RndPixelData, mFlags) == 48);
static_assert(offsetof(RndPixelData, mUnknown72) == 72);
static_assert(sizeof(RndPixelData) == 80);
