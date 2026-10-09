#include "render/lighting/RndLightMgrCom.h"

#include <array>
#include <limits>

#include "render/buffers/RndComputeBuffer.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndPixelData.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTextureArray2D.h"

namespace {

constexpr std::uint32_t kTiledLightBufferFlags = 0x12;

constexpr std::array<std::uint32_t, 7> kSpotShadowResolutions{
    256,
    512,
    1024,
    1600,
    2048,
    3200,
    4096,
};

RndComputeBuffer* NewTiledLightBuffer(
    unsigned long elementSize,
    unsigned long numElements,
    const char* name) {
    RndComputeBuffer::Description desc{};
    desc.mElementSize = elementSize;
    desc.mNumElements = numElements;
    desc.mFlags = kTiledLightBufferFlags;
    desc.mName = name;
    return RndComputeBuffer::New(desc);
}

void DeleteBuffer(RndComputeBuffer*& buffer) {
    if (buffer != nullptr) {
        delete buffer;
        buffer = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x48A400.
void RndLightMgrCom::_InitBuffers() {
    const auto& settings = *TheRndDevice()->mSettings;

    if (settings.mUseTiledLighting) {
        mPointLightBuffer = NewTiledLightBuffer(
            208,
            static_cast<unsigned long>(settings.mMaxPointLights),
            "Point Lights");
        mSpotLightBuffer = NewTiledLightBuffer(
            352,
            static_cast<unsigned long>(settings.mMaxSpotLights),
            "Spotlights");
        mDirectionalLightBuffer = NewTiledLightBuffer(
            112,
            static_cast<unsigned long>(settings.mMaxDirectionalLights),
            "Directional Lights");
        mLightProbeBuffer = NewTiledLightBuffer(
            96,
            static_cast<unsigned long>(settings.mMaxLightProbes),
            "Light Probes");

        const auto sliceZeroCapacity = settings.mMaxPointLights +
            settings.mMaxSpotLights + settings.mMaxLightProbes;
        mSliceZeroLightIdBuffer = NewTiledLightBuffer(
            sizeof(std::uint32_t),
            static_cast<unsigned long>(sliceZeroCapacity),
            "Slice Zero Ligth Ids");
    }

    _SyncSpotShadowDepthTexArray();
}

// Reconstructed from eboot.elf at 0x48AB30.
void RndLightMgrCom::_SyncSpotShadowDepthTexArray() {
    mSpotShadowDepthActive = false;
    if (mSpotShadowDepthTexArray != nullptr) {
        delete mSpotShadowDepthTexArray;
        mSpotShadowDepthTexArray = nullptr;
    }

    const auto& config = mSpotShadowConfigs[mSpotShadowConfigIndex];
    if (config.mNumLayers == 0) {
        return;
    }

    const RndDataFormatInfo depthFormat{16, 12, 0, 1, -1};
    const auto dataFormat =
        RndFindSupportedDataFormat(depthFormat, kPlatformPS4);
    const auto resolution =
        config.mResolutionIndex < kSpotShadowResolutions.size()
        ? kSpotShadowResolutions[config.mResolutionIndex]
        : std::numeric_limits<std::uint32_t>::max();

    RndTextureArray2D::Description desc;
    desc.mRequestedFormat.mUsage = 2;
    desc.mRequestedFormat.mSettings[5] = 1;
    desc.mRequestedFormat.mWrapMode = 1;
    desc.mRequestedFormat.mFilterMode = 1;
    desc.mRequestedFormat.mFlags = 2;
    desc.mName = "Spot Shadow Depth TexArray";

    const auto numLayers = static_cast<unsigned long>(config.mNumLayers);
    auto* mipChains = new RndPixelData[numLayers];
    const auto size = static_cast<int>(resolution);
    for (unsigned long index = 0; index < numLayers; ++index) {
        mipChains[index].CreateEmpty(size, size, 1, dataFormat);
    }
    desc.mPixels = {
        mipChains,
        mipChains + numLayers,
        mipChains + numLayers,
    };

    mSpotShadowDepthTexArray = RndTextureArray2D::New(desc);
    delete[] mipChains;
}

// Reconstructed from the tiled-light portion of eboot.elf at 0x480AD0.
void RndLightMgrCom::_TerminateBuffers() {
    DeleteBuffer(mPointLightBuffer);
    DeleteBuffer(mSpotLightBuffer);
    DeleteBuffer(mDirectionalLightBuffer);
    DeleteBuffer(mLightProbeBuffer);
    DeleteBuffer(mSliceZeroLightIdBuffer);
}
