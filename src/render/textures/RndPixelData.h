#pragma once

#include <cstddef>

#include "math/vector/Vector3i.h"
#include "os/memory/PoolAlloc.h"

class BinStream;
class RndPixelCanvas;
class RndPixelFormat;

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
    // Frees the level and records the format with no size. The map's
    // CreateEmpty(RndPixelFormat const&); this build takes the data format.
    void CreateEmpty(int format);  // 0x683050
    // Frees the level and records the size and format without pixels. The
    // map's format parameter is an RndPixelFormat. Not reconstructed.
    void CreateEmpty(const Vector3i& size, int format);  // 0x683170
    // Builds the mip chain below the level. Not reconstructed.
    void CreateMips();  // 0x683520
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
    // Header values that the header load (0x686650) and save (0x686410)
    // carry but nothing in this build interprets. mTileMode follows the
    // flags in the header; each level stores its own mPitch and
    // mUnprocessedBufferSize. The names are uncertain: they follow the
    // map's GetMinTiledVersion and GetUnprocessedBufferSize.
    unsigned int mTileMode;
    unsigned long mPitch;
    unsigned long mUnprocessedBufferSize;
    // The requested format read from a header older than revision 7, which
    // texture creation adopts as the description's format. Owned.
    RndPixelFormat* mLegacyFormat;

    // Releases the pixels and mips and resets the size and format. Also the
    // destructor's body.
    void Free();  // 0x682C40
};

static_assert(offsetof(RndPixelData, mSize) == 8);
static_assert(offsetof(RndPixelData, mFormat) == 20);
static_assert(offsetof(RndPixelData, mBuffer) == 24);
static_assert(offsetof(RndPixelData, mMip) == 40);
static_assert(offsetof(RndPixelData, mFlags) == 48);
static_assert(offsetof(RndPixelData, mTileMode) == 52);
static_assert(offsetof(RndPixelData, mPitch) == 56);
static_assert(offsetof(RndPixelData, mUnprocessedBufferSize) == 64);
static_assert(offsetof(RndPixelData, mLegacyFormat) == 72);
static_assert(sizeof(RndPixelData) == 80);
