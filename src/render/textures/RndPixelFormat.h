#pragma once

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
