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
// in the reference map; the binary places it in entity/Resource.o, whose
// GetCachedResourcePath uses it.
Symbol GfxApiSymbol(HxGfxApi api);  // 0x1AE4D0

// The platform ids listed in the platform_mgr config block's
// supported_platforms. The map has PlatformMgr::GetSupportedPlatforms()
// const; this build's function takes no manager object.
eastl::vector<int> GetSupportedPlatforms();  // 0x3641B0

#include "utl/messages/MsgSink.h"

// The platform services (os/PlatformMgr.o and os/PlatformMgr_PS4.o). Only
// the slots and members other reconstructed code calls are declared; the
// class is not reconstructed. The vtable is at 0x18FB748.
class PlatformMgr : public SyncMsgSource {
public:
    PlatformMgr();  // 0x36EB30
    ~PlatformMgr() override;  // slots 0-1: 0x36ECA0, 0x36ED50
    DataNode Handle(DataArray* msg, bool warn) override;  // slot 2: 0x3651B0
    virtual void PreInit();  // slot 7: 0x36E850
    virtual void Init();     // slot 8: 0x36F650
    virtual void Poll();     // slot 9: 0x36FA30

    // Creates and deletes data://write.test once, recording whether the
    // developer data volume is writable. Name not in the reference map.
    void CheckDataWritable();  // 0x372440
    // Fills the per-controller lag table, kJoypadExtraLagOffsets. Name not
    // in the reference map.
    static void InitJoypadExtraLags();  // 0x390060
};

// The platform manager, set by PreInit. At 0x19FE0B0. The PS4 half keeps
// copies at 0x19FE980 and 0x19FE988.
extern PlatformMgr* ThePlatformMgr;

// Creates the platform manager, a function-local static at 0x19FE9D0, and
// runs its PreInit. Name not in the reference map.
void PlatformMgrPreInit();  // 0x36E7E0

// The component that exposes the platform to scripts ("Allows access to the
// current platform"). Only its registration is declared.
class PlatformMgrCom {
public:
    static void Init();  // 0x3694A0
};
