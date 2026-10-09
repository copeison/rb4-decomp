#include "render/system/RndWindow.h"

#include <limits>

#include "os/memory/MemMgr.h"
#include "render/core/targets/render_target_resources.h"
#include "render/core/targets/render_target_resources_lifecycle.h"

using namespace rb4;

namespace {

static_assert(
    offsetof(RenderTargetState, state_flags) ==
    offsetof(RenderTargetResources, flags));
static_assert(
    offsetof(RenderTargetState, draw_mode) ==
    offsetof(RenderTargetResources, reserved_10));
static_assert(
    offsetof(RenderTargetState, width) ==
    offsetof(RenderTargetResources, extent));

RenderTargetResources& TargetResources(RenderTargetState& state) {
    return reinterpret_cast<RenderTargetResources&>(state);
}

// Reconstructed from eboot.elf at 0x6B40A0. The 2D buffer collection's
// constructor.
void ConstructBuffers(RenderTargetState& state, unsigned int flags) {
    auto& resources = TargetResources(state);
    render_target_resources_construct(resources, flags, 0);
    render_target_resources_set_concrete_dispatch(resources);
}

// Reconstructed from eboot.elf at 0x6B40E0. The 2D buffer collection's
// deleting destructor.
void DeleteBuffers(RenderTargetState& state) {
    render_target_resources_destruct(TargetResources(state));
    MemFree(&state);
}

}  // namespace

// Reconstructed from eboot.elf at 0x4486F0.
RndWindow::RndWindow() : mUnknown8(-1) {}

// Reconstructed from eboot.elf at 0x448710.
RndWindow::~RndWindow() {}

// Reconstructed from eboot.elf at 0x448820.
void RndWindow::Poll() {}

// Reconstructed from eboot.elf at 0x448830.
void RndWindow::CheckForResize() {}

// Reconstructed from eboot.elf at 0x448840.
bool RndWindow::_UnknownSlot6() {
    return true;
}

// Reconstructed from eboot.elf at 0x448730.
RenderExtent RndWindow::GetSize() const {
    const auto buffers = GetBufferCollections();
    if (buffers.count == 0) {
        return {};
    }
    const auto& first = *buffers.states[0];
    return {first.width, first.height};
}

// Reconstructed from eboot.elf at 0x448760.
unsigned int RndWindow::GetShadingMode() const {
    const auto buffers = GetBufferCollections();
    return buffers.count == 0
        ? std::numeric_limits<unsigned int>::max()
        : buffers.states[0]->draw_mode;
}

// Reconstructed from eboot.elf at 0x448780.
void RndWindow::SetShadingMode(unsigned int mode) {
    const auto buffers = GetBufferCollections();
    for (std::size_t i = 0; i < buffers.count; ++i) {
        buffers.states[i]->draw_mode = mode;
    }
}

// Reconstructed from eboot.elf at 0x4487C0.
unsigned int RndWindow::GetBufferInspectionMode() const {
    const auto buffers = GetBufferCollections();
    return buffers.count == 0
        ? std::numeric_limits<unsigned int>::max()
        : buffers.states[0]->debug_view;
}

// Reconstructed from eboot.elf at 0x4487E0.
void RndWindow::SetBufferInspectionMode(unsigned int mode) {
    const auto buffers = GetBufferCollections();
    for (std::size_t i = 0; i < buffers.count; ++i) {
        buffers.states[i]->debug_view = mode;
    }
}

// Reconstructed from eboot.elf at 0x11B2CD0.
RndBufferedWindow::RndBufferedWindow(
    unsigned int bufferFlags,
    bool createBuffers)
    : mOwnsBuffers(createBuffers),
      mBuffers(nullptr),
      mActiveBuffers(nullptr) {
    if (createBuffers) {
        auto* buffers = static_cast<RenderTargetState*>(
            operator new(sizeof(RenderTargetResources)));
        ConstructBuffers(*buffers, bufferFlags);
        mBuffers = buffers;
        mActiveBuffers = buffers;
    }
}

// Reconstructed from eboot.elf at 0x11B2D40.
RndBufferedWindow::~RndBufferedWindow() {
    if (mOwnsBuffers) {
        if (mBuffers != nullptr) {
            DeleteBuffers(*mBuffers);
        }
        mBuffers = nullptr;
        mActiveBuffers = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x11B2DE0.
unsigned long RndBufferedWindow::_GetActiveBufferIndex() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x11B2DF0.
RenderTargetStateHandle RndBufferedWindow::GetBufferCollections() const {
    return {const_cast<RenderTargetState**>(&mActiveBuffers), 1};
}

// Reconstructed from eboot.elf at 0x11B2E00.
void RndBufferedWindow::SetBufferCollection(RenderTargetState* buffers) {
    mBuffers = buffers;
    mActiveBuffers = buffers;
}
