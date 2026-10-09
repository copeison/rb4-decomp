#include "render/system/RndWindow.h"

#include <limits>

#include "render/targets/RndBufferCollection.h"

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
Vector2i RndWindow::GetSize() const {
    const auto buffers = GetBufferCollections();
    if (buffers.mCount == 0) {
        return {};
    }
    const auto& size = buffers.mCollections[0]->mSize;
    return {size.x, size.y};
}

// Reconstructed from eboot.elf at 0x448760.
unsigned int RndWindow::GetShadingMode() const {
    const auto buffers = GetBufferCollections();
    return buffers.mCount == 0
        ? std::numeric_limits<unsigned int>::max()
        : buffers.mCollections[0]->mShadingMode;
}

// Reconstructed from eboot.elf at 0x448780.
void RndWindow::SetShadingMode(unsigned int mode) {
    const auto buffers = GetBufferCollections();
    for (std::size_t i = 0; i < buffers.mCount; ++i) {
        buffers.mCollections[i]->mShadingMode = mode;
    }
}

// Reconstructed from eboot.elf at 0x4487C0.
unsigned int RndWindow::GetBufferInspectionMode() const {
    const auto buffers = GetBufferCollections();
    return buffers.mCount == 0
        ? std::numeric_limits<unsigned int>::max()
        : buffers.mCollections[0]->mBufferInspectionMode;
}

// Reconstructed from eboot.elf at 0x4487E0.
void RndWindow::SetBufferInspectionMode(unsigned int mode) {
    const auto buffers = GetBufferCollections();
    for (std::size_t i = 0; i < buffers.mCount; ++i) {
        buffers.mCollections[i]->mBufferInspectionMode = mode;
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
        auto* buffers = new RndBufferCollection2D(bufferFlags, 0);
        mBuffers = buffers;
        mActiveBuffers = buffers;
    }
}

// Reconstructed from eboot.elf at 0x11B2D40.
RndBufferedWindow::~RndBufferedWindow() {
    if (mOwnsBuffers) {
        delete mBuffers;
        mBuffers = nullptr;
        mActiveBuffers = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x11B2DE0.
unsigned long RndBufferedWindow::_GetActiveBufferIndex() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x11B2DF0.
RndBufferCollectionList RndBufferedWindow::GetBufferCollections() const {
    return {const_cast<RndBufferCollection**>(&mActiveBuffers), 1};
}

// Reconstructed from eboot.elf at 0x11B2E00.
void RndBufferedWindow::SetBufferCollection(RndBufferCollection* buffers) {
    mBuffers = buffers;
    mActiveBuffers = buffers;
}
