#include "renderps4/video/PS4Window.h"

#include <algorithm>

#include "renderps4/system/PS4Device.h"

using namespace rb4;

namespace {

constexpr std::size_t kBackBufferCount = 2;
constexpr std::size_t kMinimumBackBufferAlignment = 64 * 1024;
constexpr unsigned int kBufferFlags = 0x40007FFF;
constexpr const char* kBackBufferAllocationName = "BackBuffer";

}  // namespace

// Reconstructed from eboot.elf at 0x8E24A0.
PS4Window::PS4Window()
    : RndBufferedWindow(kBufferFlags, true),
      mActiveBuffer(0) {
    _InitBuffers();
}

// Reconstructed from eboot.elf at 0x8E24A0 (inlined into the constructor).
void PS4Window::_InitBuffers() {
    auto& device = *gPS4Device;
    const auto specification = _BackBufferSpecification(
        device, OrbisBackBufferDataFormat::kB8G8R8A8Srgb);
    OrbisGpuRenderTarget targets[kBackBufferCount] = {};
    for (auto& target : targets) {
        _InitRenderTarget(target, specification);
        const auto sizeAlign = _RenderTargetSizeAlign(target);
        const auto alignment =
            std::max(sizeAlign.alignment, kMinimumBackBufferAlignment);
        auto* storage = _AllocateBackBuffer(
            sizeAlign.size, kBackBufferAllocationName, alignment);
        _SetRenderTargetStorage(target, storage);
        _DisableAuxiliarySurfaces(target);
    }

    auto* texture = _WrapBackBufferTextures(targets, kBackBufferCount);
    _AttachBackBufferTexture(*texture);
    _RegisterBackBuffers(device, targets, kBackBufferCount);
}

// Reconstructed from eboot.elf at 0x8E2860.
PS4Window::~PS4Window() {}

// Reconstructed from eboot.elf at 0x8E2890.
bool PS4Window::AdvanceFrame() {
    mActiveBuffer = (mActiveBuffer & 1U) == 0 ? 1 : 0;
    return mActiveBuffer != 0;
}
