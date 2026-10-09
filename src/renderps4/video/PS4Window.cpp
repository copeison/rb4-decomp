#include "renderps4/video/PS4Window.h"

#include <algorithm>

#include "render/platform/orbis/video/orbis_back_buffer_adapters.h"
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
    auto& device = *gPS4Device;
    const auto specification = orbis_back_buffer_specification(
        device, OrbisBackBufferDataFormat::kB8G8R8A8Srgb);
    OrbisGpuRenderTarget targets[kBackBufferCount] = {};
    for (auto& target : targets) {
        orbis_gpu_render_target_initialize(target, specification);
        const auto sizeAlign = orbis_gpu_render_target_size_align(target);
        const auto alignment =
            std::max(sizeAlign.alignment, kMinimumBackBufferAlignment);
        auto* storage = orbis_gpu_allocate_named(
            sizeAlign.size, kBackBufferAllocationName, alignment);
        orbis_gpu_render_target_set_storage(target, storage);
        orbis_gpu_render_target_disable_auxiliary_surfaces(target);
    }

    auto* texture = orbis_wrap_back_buffer_textures(targets, kBackBufferCount);
    orbis_back_buffer_attach_texture(*this, *texture);
    orbis_video_output_register_back_buffers(device, targets, kBackBufferCount);
}

// Reconstructed from eboot.elf at 0x8E2860.
PS4Window::~PS4Window() {}

// Reconstructed from eboot.elf at 0x8E2890.
bool PS4Window::AdvanceFrame() {
    mActiveBuffer = (mActiveBuffer & 1U) == 0 ? 1 : 0;
    return mActiveBuffer != 0;
}
