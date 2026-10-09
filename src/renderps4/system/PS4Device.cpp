#include "renderps4/system/PS4Device.h"

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/platform/orbis/video/orbis_back_buffer.h"
#include "renderps4/context/PS4Context.h"

using namespace rb4;

PS4Device* gPS4Device = nullptr;

namespace {

constexpr const char* kUnknownThreadName = "Unknown Thread!";

PS4DeferredDelete* DeferredDeleteAnchor(PS4DeferredDeleteList& list) {
    return reinterpret_cast<PS4DeferredDelete*>(&list);
}

void EraseDeferredDelete(PS4DeferredDeleteList& list, PS4DeferredDelete& node) {
    node.mNext->mPrev = node.mPrev;
    node.mPrev->mNext = node.mNext;
    list.mAllocator.deallocate(&node, sizeof(node));
    --list.mSize;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D77F0.
PS4Device::PS4Device()
    : mSubmitToken(1),
      mDefaultVertexBuffer(nullptr) {
    mSubmitThread.Init(kUnknownThreadName);
    auto* anchor = DeferredDeleteAnchor(mDeferredDeletes);
    mDeferredDeletes.mNext = anchor;
    mDeferredDeletes.mPrev = anchor;
    mDeferredDeletes.mSize = 0;
    mFlipRate = -1;
    gPS4Device = this;
}

// Reconstructed from eboot.elf at 0x8D79B0.
PS4Device::~PS4Device() {
    gPS4Device = nullptr;
    auto* anchor = DeferredDeleteAnchor(mDeferredDeletes);
    for (auto* node = mDeferredDeletes.mNext; node != anchor;) {
        auto* next = node->mNext;
        mDeferredDeletes.mAllocator.deallocate(node, sizeof(*node));
        node = next;
    }
    mSubmitThread.mThread._ForceKillThread();
}

// Reconstructed from eboot.elf at 0x8D8580.
void PS4Device::_PostInitImpl() {}

// Reconstructed from eboot.elf at 0x8D8590.
int PS4Device::_GetGpuBlockingBehaviorImpl() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x8D83E0.
int PS4Device::_UnknownSlot14Impl() {
    return 0;
}

// Reconstructed from eboot.elf at 0x8D8100.
void PS4Device::_BeginFrameImpl(bool) {
    WaitForIdle();
    _ReleaseRetiredAllocations();
    if (mBeginFramePending) {
        _FlushPendingBeginFrame();
    }
    orbis_render_context_reset_active_frame(Context());
}

// Reconstructed from eboot.elf at 0x8D8140.
void PS4Device::WaitForIdle() {
    while (!orbis_render_context_submissions_complete(Context())) {
        scePthreadYield();
        if (mBeginFramePending) {
            _FlushPendingBeginFrame();
        }
    }
}

// Reconstructed from eboot.elf at 0x8D8200. Allocations retired two or more
// frames ago are freed.
void PS4Device::_ReleaseRetiredAllocations() {
    if (mFrameCount < 2) {
        return;
    }
    const auto completed = mFrameCount - 2;
    ScopedCritSec lock(mDeferredDeleteCritSec);
    auto* anchor = DeferredDeleteAnchor(mDeferredDeletes);
    for (auto* node = mDeferredDeletes.mNext; node != anchor;) {
        auto* next = node->mNext;
        if (node->mFrame <= completed) {
            MemFree(node->mAllocation);
            EraseDeferredDelete(mDeferredDeletes, *node);
        }
        node = next;
    }
}

// Reconstructed from eboot.elf at 0x8D8300.
void PS4Device::_EndFrameImpl(
    FixedVector<RenderFrameOwner*, 6>& windows,
    bool) {
    // The submission lock stays held while the frame is submitted; only the
    // token wait is counted as an entry.
    scePthreadMutexLock(&mSubmitCritSec.mCritSec);
    ++mSubmitCritSec.mEntryCount;
    while (mSubmitToken == 0) {
        mSubmitCondition.Wait();
    }
    mSubmitToken = 0;
    --mSubmitCritSec.mEntryCount;
    if (mBeginFramePending) {
        _FlushPendingBeginFrame();
    }

    orbis_render_context_submit_frame(Context());
    for (unsigned long i = 0; i < windows.mSize; ++i) {
        orbis_back_buffer_advance(
            *reinterpret_cast<OrbisBackBuffer*>(windows.mData[i]));
    }
    scePthreadMutexUnlock(&mSubmitCritSec.mCritSec);
}

// Reconstructed from eboot.elf at 0x8D83F0.
void PS4Device::DeferredDelete(void* allocation) {
    if (allocation == nullptr) {
        return;
    }
    ScopedCritSec lock(mDeferredDeleteCritSec);
    auto* anchor = DeferredDeleteAnchor(mDeferredDeletes);
    auto* node = static_cast<PS4DeferredDelete*>(
        mDeferredDeletes.mAllocator.allocate(sizeof(PS4DeferredDelete)));
    node->mAllocation = allocation;
    node->mFrame = TheRndDevice()->mFrameCount;
    node->mNext = anchor;
    node->mPrev = mDeferredDeletes.mPrev;
    mDeferredDeletes.mPrev->mNext = node;
    mDeferredDeletes.mPrev = node;
    ++mDeferredDeletes.mSize;
}

// Reconstructed from eboot.elf at 0x8D84B0.
void PS4Device::_ProcessDeferredDeletion() {
    ScopedCritSec lock(mDeferredDeleteCritSec);
    auto* anchor = DeferredDeleteAnchor(mDeferredDeletes);
    for (auto* node = mDeferredDeletes.mNext; node != anchor;) {
        auto* next = node->mNext;
        MemFree(node->mAllocation);
        EraseDeferredDelete(mDeferredDeletes, *node);
        node = next;
    }
}
