#include "render/platform/orbis/video/orbis_back_buffer.h"

#include <algorithm>
#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/core/targets/render_target.h"
#include "renderps4/system/PS4Device.h"
#include "render/platform/orbis/video/orbis_back_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kBackBufferSize = 40;
constexpr std::size_t kBackBufferCount = 2;
constexpr std::size_t kMinimumBackBufferAlignment = 64 * 1024;
constexpr std::uint32_t kRenderTargetFlags = 0x40007FFF;
constexpr const char* kBackBufferAllocationName = "BackBuffer";

}  // namespace

static_assert(sizeof(OrbisBackBuffer) == kBackBufferSize,
              "unexpected Orbis back-buffer size");
static_assert(offsetof(OrbisBackBuffer, active_buffer) == 32,
              "unexpected active back-buffer offset");
static_assert(sizeof(OrbisGpuRenderTarget) == 64,
              "unexpected Gnm render-target size");

OrbisBackBuffer* orbis_back_buffer_create(PS4Device& system) {
    auto* storage = operator new(kBackBufferSize);
    auto* back_buffer = static_cast<OrbisBackBuffer*>(storage);
    orbis_back_buffer_construct(*back_buffer, system);
    system._InstallMainWindow(reinterpret_cast<RenderFrameOwner*>(&*back_buffer));
    return back_buffer;
}

// Reconstructed from eboot.elf at 0x8E24A0.
void orbis_back_buffer_construct(
    OrbisBackBuffer& back_buffer,
    PS4Device& system) {
    render_target_construct(
        back_buffer, kRenderTargetFlags, true);
    orbis_back_buffer_install_vtable(back_buffer);
    back_buffer.active_buffer = 0;

    const auto specification = orbis_back_buffer_specification(
        system, OrbisBackBufferDataFormat::kB8G8R8A8Srgb);
    OrbisGpuRenderTarget targets[kBackBufferCount] = {};
    for (auto& target : targets) {
        orbis_gpu_render_target_initialize(target, specification);
        const auto size_align = orbis_gpu_render_target_size_align(target);
        const auto alignment = std::max(
            size_align.alignment, kMinimumBackBufferAlignment);
        auto* storage = orbis_gpu_allocate_named(
            size_align.size, kBackBufferAllocationName, alignment);
        orbis_gpu_render_target_set_storage(target, storage);
        orbis_gpu_render_target_disable_auxiliary_surfaces(target);
    }

    auto* texture = orbis_wrap_back_buffer_textures(
        targets, kBackBufferCount);
    orbis_back_buffer_attach_texture(back_buffer, *texture);
    orbis_video_output_register_back_buffers(
        system, targets, kBackBufferCount);
}

// Reconstructed from the thunk at 0x8E2860.
void orbis_back_buffer_destruct(OrbisBackBuffer& back_buffer) {
    render_target_destruct(back_buffer);
}

// Reconstructed from eboot.elf at 0x8E2870.
void orbis_back_buffer_delete(OrbisBackBuffer& back_buffer) {
    orbis_back_buffer_destruct(back_buffer);
    operator delete(&back_buffer);
}

// Reconstructed from eboot.elf at 0x8E2890.
bool orbis_back_buffer_advance(OrbisBackBuffer& back_buffer) {
    back_buffer.active_buffer = (back_buffer.active_buffer & 1U) == 0 ? 1 : 0;
    return back_buffer.active_buffer != 0;
}

}  // namespace rb4
