#include "renderps4/video/PS4Window.h"

#include <algorithm>
#include <cstdint>
#include <gnm/dataformats.h>
#include <gnm/platform.h>
#include <gpu_address.h>
#include <video_out.h>

#include "render/system/RndConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/textures/PS4Texture2D.h"

namespace {

constexpr int kNumBackBuffers = 2;
constexpr std::uint32_t kMinimumBackBufferAlignment = 64 * 1024;
constexpr unsigned int kBufferFlags = 0x40007FFF;

// Allocates a back buffer for the target and points the target at it,
// without CMASK or FMASK. Inlined twice into the constructor. Name not in
// the reference map.
void AllocateBackBuffer(sce::Gnm::RenderTarget& target) {
    const auto sizeAlign = target.getColorSizeAlign();
    const auto alignment = std::max<std::uint32_t>(sizeAlign.m_align, kMinimumBackBufferAlignment);
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    void* surface = MemAlloc(sizeAlign.m_size, "BackBuffer", static_cast<int>(alignment));
    MemPopHeap();
    target.setBaseAddress(surface);
    target.setCmaskAddress256ByteBlocks(0);
    target.setFmaskAddress256ByteBlocks(0);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8E24A0. The two back buffers are sRGB
// BGRA render targets at the configured output resolution, which defaults to
// the platform's largest supported resolution on PS4 Pro and its smallest
// otherwise. They are wrapped in one texture, installed as the window's back
// buffer and registered with video output.
PS4Window::PS4Window()
    : RndBufferedWindow(kBufferFlags, true),
      mActiveBuffer(0) {
    const auto gpuMode = sce::Gnm::getGpuMode();
    auto* settings = TheRndDevice()->mSettings;
    if (!settings->mResolutionOverridden) {
        const auto& resolutions = TheRndDevice()->mCapabilities[kPlatformPS4].mResolutions;
        settings->mOutputResolution =
            gpuMode != sce::Gnm::kGpuModeBase ? resolutions.back() : resolutions.front();
    }

    sce::Gnm::TileMode tileMode;
    sce::GpuAddress::computeSurfaceTileMode(
        gpuMode,
        &tileMode,
        sce::GpuAddress::kSurfaceTypeColorTargetDisplayable,
        sce::Gnm::kDataFormatB8G8R8A8UnormSrgb,
        1);
    sce::Gnm::RenderTargetSpec spec;
    spec.init();
    spec.m_width = static_cast<std::uint32_t>(settings->mOutputResolution.x);
    spec.m_height = static_cast<std::uint32_t>(settings->mOutputResolution.y);
    spec.m_pitch = 0;
    spec.m_numSlices = 1;
    spec.m_colorFormat = sce::Gnm::kDataFormatB8G8R8A8UnormSrgb;
    spec.m_colorTileModeHint = tileMode;
    spec.m_minGpuMode = gpuMode;
    spec.m_numSamples = sce::Gnm::kNumSamples1;
    spec.m_numFragments = sce::Gnm::kNumFragments1;

    sce::Gnm::RenderTarget* targets[kNumBackBuffers];
    for (auto*& target : targets) {
        target = new sce::Gnm::RenderTarget();
        target->init(&spec);
        AllocateBackBuffer(*target);
    }

    mBuffers->InstallBackBuffer(PS4Texture2D::CreateAsBackBuffer(targets), nullptr);

    SceVideoOutBufferAttribute attribute = {};
    sceVideoOutSetBufferAttribute(
        &attribute,
        SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
        SCE_VIDEO_OUT_TILING_MODE_TILE,
        SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
        targets[0]->getWidth(),
        targets[0]->getHeight(),
        targets[0]->getPitch());
    attribute.option = SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_OPTION_STRICT_COLORIMETRY;
    void* addresses[kNumBackBuffers] = {
        targets[0]->getBaseAddress(),
        targets[1]->getBaseAddress(),
    };
    sceVideoOutRegisterBuffers(gPS4Device->mVideoOutHandle, 0, addresses, kNumBackBuffers, &attribute);
}

// Reconstructed from eboot.elf at 0x8E2860.
PS4Window::~PS4Window() {}

// Reconstructed from eboot.elf at 0x8E2890.
bool PS4Window::AdvanceFrame() {
    mActiveBuffer = (mActiveBuffer & 1U) == 0 ? 1 : 0;
    return mActiveBuffer != 0;
}
