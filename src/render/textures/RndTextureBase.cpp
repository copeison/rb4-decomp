#include "render/textures/RndTextureBase.h"

#include "os/files/File.h"

// Reconstructed from eboot.elf at 0x69B930.
RndTextureBase::Description::Description()
    : mType(-1),
      mRequestedFormat{},
      mFormat{},
      mDataFormat(-1),
      mWidth(0),
      mHeight(0),
      mDepth(0),
      mUnknown108(0),
      mSourceData(nullptr),
      mSourceSize(0),
      mKeepPixelData(false),
      mBindlessIndex(-1),
      mResourceFlags(0),
      mName(nullptr) {}

// Reconstructed from eboot.elf at 0x6A4770. Usages 0, 8, and 9 are the
// default-like usages; usage 9 forces its own filter, wrap, and settings.
void RndTextureBase::Description::ResolveFormat(int type, long fallback) {
    auto& format = mFormat;
    const auto& defaults = mRequestedFormat;
    format.mUsage = defaults.mUsage;
    for (int index = 0; index < 7; ++index) {
        if (format.mSettings[index] == 0) {
            format.mSettings[index] = defaults.mSettings[index];
        }
    }
    if (format.mWrapMode == 0) {
        format.mWrapMode = defaults.mWrapMode;
    }
    if (format.mFilterMode == 0) {
        format.mFilterMode = defaults.mFilterMode;
    }
    format.mFlags |= defaults.mFlags;

    const auto usage = format.mUsage;
    const auto defaultLike = usage == 0 || usage == 8 || usage == 9;
    const auto standard = usage == 0 || usage == 8;
    auto& settings = format.mSettings;
    if (settings[3] == 0) {
        settings[3] = defaultLike ? 2 : 1;
    }
    if (usage == 9) {
        settings[3] = 2;
    }
    if (settings[5] == 0) {
        settings[5] = usage == 8 ? 1 : 2;
    }
    if (format.mWrapMode == 0) {
        const auto cube = (type | 4) == 7;
        format.mWrapMode = cube ? 1 : (usage == 8 ? 1 : 2);
    }
    if (format.mFilterMode == 0) {
        format.mFilterMode = settings[5] == 1 ? 2 : 3;
    }

    const auto resolvedFallback = fallback == -1 ? 4 : fallback;
    if (settings[0] == 0 && standard) {
        settings[0] = resolvedFallback == 4 ? 3 : 1;
    }
    if (settings[1] == 0) {
        settings[1] = resolvedFallback == 4 ? (standard ? 3 : 1) : 1;
    }
    if (settings[4] == 0) {
        settings[4] = (format.mFlags & 3U) == 0 ? 2 : 1;
    }

    if (usage == 9) {
        settings[0] = 1;
        if ((format.mFlags & 2U) != 0) {
            settings[4] = 1;
            settings[5] = 1;
            format.mFilterMode = 2;
        } else {
            format.mFlags |= 4U;
            settings[4] = 2;
            settings[5] = 2;
            format.mFilterMode = 3;
        }
        format.mWrapMode = (type | 4) == 7 ? 1 : 2;
    }
}

bool RndTextureBase::Description::ShouldKeepPixelData() const {
    return mKeepPixelData || (mFormat.mFlags & 5U) != 0;
}

// Reconstructed from eboot.elf at 0x69B6E0.
RndTextureBase::RndTextureBase() : mResourceIndex(-1) {}

// Reconstructed from eboot.elf at 0x690500.
int RndTextureBase::_GetTypeImpl() const {
    return mBaseDesc.mType;
}

// Reconstructed from eboot.elf at 0x697870.
RndTextureBase* RndTextureBase::_GetLinkedTextureImpl(long&) {
    return this;
}

// Reconstructed from eboot.elf at 0x690510.
bool RndTextureBase::_SyncStaticFromStreamImpl(BinStream&) {
    return false;
}

// Reconstructed from eboot.elf at 0x697880.
void RndTextureBase::_LoadBuffersImpl(BinStream&) {}

// Reconstructed from eboot.elf at 0x69B7A0. Pixel data is freed after the
// upload unless the description asks to keep it.
void RndTextureBase::SyncStatic(const RndTextureBase* reuse) {
    if (gResourcePrecacheMode) {
        return;
    }
    _SyncStaticImpl(reuse);
    if (!mBaseDesc.mKeepPixelData && (mBaseDesc.mFormat.mFlags & 5U) == 0) {
        _FreePixelDataImpl();
    }
}

// Reconstructed from eboot.elf at 0x50CE00.
int TextureDefaultWrapMode(unsigned int kind) {
    constexpr unsigned long kWrapModeOneKinds = 0x1F80400E0UL;
    if (kind > 32) {
        return -1;
    }
    return (kWrapModeOneKinds & (1UL << kind)) != 0 ? 1 : -1;
}

// Reconstructed from eboot.elf at 0x50CE30.
int TextureDefaultFilterMode(unsigned int kind) {
    constexpr unsigned long kFilterModeTwoKinds = 0x178040060UL;
    constexpr unsigned long kFilterModeOneKinds = 0x80000080UL;
    if (kind > 32) {
        return -1;
    }
    const auto bit = 1UL << kind;
    if ((kFilterModeTwoKinds & bit) != 0) {
        return 2;
    }
    return (kFilterModeOneKinds & bit) != 0 ? 1 : -1;
}
