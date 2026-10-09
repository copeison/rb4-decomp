#pragma once

#include "os/platform/PlatformMgr.h"

// Texture format request: usage, seven format settings, wrap and filter
// modes, and flags. A texture description holds the requested format and
// the format resolved from it. Field names are not in the reference map.
class RndPixelFormat {
public:
    int mUsage;
    unsigned int mSettings[7];
    unsigned int mWrapMode;
    unsigned int mFilterMode;
    unsigned int mFlags;
};

static_assert(sizeof(RndPixelFormat) == 44);

// Texture usages the reconstructed code distinguishes. Names not in the
// reference map.
enum RndTextureUsage : int {
    kTextureUsageDefault = 0,
    kTextureUsageDepth = 2,
    kTextureUsageTiledLighting = 10,
};

// A data format's bit width, channel order, storage type, gamma and block
// compression. Name not in the reference map; the fields follow the map's
// RndPixelFormat::Order, Storage, Gamma and Compression enumerations.
struct RndDataFormatInfo {
    unsigned int mBitsPerPixel;
    unsigned int mOrder;    // Channel order; -1 for block formats.
    unsigned int mStorage;  // 0 unorm, 1 snorm, 2 float; -1 for blocks.
    unsigned int mGamma;    // 1 linear, 2 sRGB.
    int mCompression;       // 0 none, -1 any, positive for block formats.
};

static_assert(sizeof(RndDataFormatInfo) == 20);

// Data formats are indices into the engine's format list; -1 is invalid.
// Names not in the reference map.
// Describes a data format.
RndDataFormatInfo RndGetDataFormatInfo(int dataFormat);  // 0x68DB80
// The data format matching a description exactly, or -1.
int RndFindDataFormat(const RndDataFormatInfo& info);  // 0x68E070
// The data format matching a description that the platform supports,
// falling back to related channel orders, storage types and packed depth
// formats; -1 when none is supported. The map's counterpart is
// RndPixelFormat::SetToSupportedFormat().
int RndFindSupportedDataFormat(
    const RndDataFormatInfo& info,
    HxPlatform platform);  // 0x68E4D0, 0x68E550
