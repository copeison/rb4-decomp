#include "os/platform/PlatformMgr.h"

#include <array>
#include <cstddef>

#include "os/system/System.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"

// Reconstructed from eboot.elf at 0x363030.
Symbol PlatformSymbol(HxPlatform platform) {
    static Symbol sNames[kNumPlatforms] = {
        Symbol(""),
        Symbol(""),
        Symbol(""),
        Symbol("pc"),
        Symbol(""),
        Symbol("xb1"),
        Symbol(""),
        Symbol("ps4"),
        Symbol("android"),
        Symbol("ios"),
        Symbol("osx"),
        Symbol("tvos"),
        Symbol("nx"),
    };
    return platform < kNumPlatforms ? sNames[platform] : Symbol();
}

// Reconstructed from eboot.elf at 0x3641B0.
eastl::vector<int> GetSupportedPlatforms() {
    static Symbol sPlatformMgr;
    if (sPlatformMgr == Symbol()) {
        sPlatformMgr = Symbol("platform_mgr");
    }
    static Symbol sSupportedPlatforms;
    if (sSupportedPlatforms == Symbol()) {
        sSupportedPlatforms = Symbol("supported_platforms");
    }
    auto* config = SystemConfig(sPlatformMgr, sSupportedPlatforms);
    eastl::vector<int> platforms;
    platforms.reserve(config->Size() - 1);
    for (int index = 1; index < config->Size(); ++index) {
        platforms.push_back(config->Int(index));
    }
    return platforms;
}

// Reconstructed from eboot.elf at 0x363FE0.
DataArray* MakeDataArray(const DataNode& node) {
    auto* array = new DataArray(1);
    array->Node(0) = node;
    return array;
}
