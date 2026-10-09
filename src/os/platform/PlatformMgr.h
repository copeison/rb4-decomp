#pragma once

#include <cstdint>

#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// The engine's platform enumeration. Name in the reference map; the
// enumerator names are not. Values 1, 2, 4 and 6 have empty names in this
// build's platform table.
enum HxPlatform : std::uint32_t {
    kPlatformNone = 0,
    kPlatformPC = 3,
    kPlatformXB1 = 5,
    kPlatformPS4 = 7,
    kPlatformAndroid = 8,
    kPlatformIOS = 9,
    kPlatformOSX = 10,
    kPlatformTVOS = 11,
    kPlatformNX = 12,
    kNumPlatforms = 13,
};

// The graphics APIs a platform's configuration can select. Names not in the
// reference map.
enum HxGfxApi : std::uint32_t {
    kGfxApiNull = 0,
    kGfxApiDX11 = 1,
    kGfxApiPS4 = 2,
    kGfxApiMetal = 3,
    kGfxApiVulkan = 4,
    kGfxApiNX = 5,
    kGfxApiGLES3 = 6,
    kNumGfxApis = 7,
};

// The platform's short name ("ps4"), or the empty symbol for a platform
// without one.
Symbol PlatformSymbol(HxPlatform platform);  // 0x363030

// The graphics API's short name ("ps4"), or "" when out of range. Name not
// in the reference map; its object file is not identified.
Symbol GfxApiSymbol(HxGfxApi api);  // 0x1AE4D0

// The platform ids listed in the platform_mgr config block's
// supported_platforms. The map has PlatformMgr::GetSupportedPlatforms()
// const; this build's function takes no manager object.
eastl::vector<int> GetSupportedPlatforms();  // 0x3641B0
