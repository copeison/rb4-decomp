#include "renderps4/system/PS4Device.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <kernel/equeue.h>
#include <system_service.h>
#include <video_out.h>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/system/RndConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Factory.h"
#include "renderps4/textures/PS4Texture2D.h"
#include "renderps4/video/PS4Window.h"
#include "utl/time/Timer.h"

extern "C" {

std::int32_t sceGnmAddEqEvent(
    SceKernelEqueue queue,
    std::uint32_t event_id,
    void* user_data);
std::int32_t sceGnmDeleteEqEvent(
    SceKernelEqueue queue,
    std::uint32_t event_id);
std::int32_t sceGnmSubmitDone();

}

using namespace rb4;

PS4Device* gPS4Device = nullptr;

// State the submit-done thread keeps across events. Name not in the reference
// map.
struct PS4SubmitDoneState {
    unsigned long mNextBuffer = 0;
    long mPreviousBuffer = 2;
    std::uint64_t mLastSubmitCheck = 0;
    std::uint64_t mPendingSubmitTicks = 0;
};

namespace {

constexpr const char* kUnknownThreadName = "Unknown Thread!";
constexpr const char* kEventQueueName = "EOP QUEUE";
constexpr const char* kSubmitThreadName = "SubmitDoneThread";
constexpr std::uint32_t kGnmEventId = 64;
constexpr auto kSubmitThreadPriority = static_cast<Thread::ThreadPriority>(699);
constexpr float kSubmitDoneTimeoutMilliseconds = 1000.0F;
constexpr unsigned long kBackBufferCount = 2;

template <typename Callback>
void ForEachOutputTexture(PS4Device& device, Callback callback) {
    const auto states = device.mMainWindow->GetBufferCollections();
    for (std::size_t index = 0; index < states.mCount; ++index) {
        auto* texture = states.mCollections[index]->mBackBuffer;
        if (texture != nullptr) {
            callback(reinterpret_cast<PS4Texture2D&>(*texture));
        }
    }
}

std::uint32_t VideoFlipMode(std::int32_t rate) {
    return rate == 0
        ? SCE_VIDEO_OUT_FLIP_MODE_HSYNC
        : SCE_VIDEO_OUT_FLIP_MODE_WINDOW_2;
}

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
    FixedVector<RndWindow*, 6>& windows,
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
        (reinterpret_cast<PS4Window*>(windows.mData[i]))->AdvanceFrame();
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

// Reconstructed from eboot.elf at 0x8D7B20.
void PS4Device::_InitImpl(const RndInitParams*) {
    _InitVideoOutput();

    _InitDefaultVertexBuffers();
    _InitIdentityInstanceBuffers();
    _InstallFactory(new PS4Factory);
    _InstallMainWindow(new PS4Window);
    static_cast<void>(orbis_render_context_create(*this));

    mSubmitCondition.Init(mSubmitCritSec);
    mSubmitThread.Create(
        _SubmitDoneThreadEntry,
        this,
        kSubmitThreadName,
        -1,
        kSubmitThreadPriority,
        0,
        0);
    mSubmitThreadRunning = true;
    mSubmitToken = 0;
    mSubmitThread.mThread.Start();
    _WaitForSubmitThread();
    sceSystemServiceHideSplashScreen();
}

// Reconstructed from eboot.elf at 0x8D7B20 (inlined into _InitImpl).
void PS4Device::_InitVideoOutput() {
    constexpr std::int32_t kSystemUserId = 255;
    mVideoOutHandle = sceVideoOutOpen(kSystemUserId, 0, 0, nullptr);
    sceVideoOutSetFlipRate(mVideoOutHandle, 0);
    sceVideoOutSetWindowModeMargins(mVideoOutHandle, 1080, 0);

    SceKernelEqueue queue = nullptr;
    sceKernelCreateEqueue(&queue, kEventQueueName);
    mEventQueue = queue;
    sceGnmAddEqEvent(mEventQueue, kGnmEventId, nullptr);
    sceVideoOutAddFlipEvent(mEventQueue, mVideoOutHandle, nullptr);
}

// Reconstructed from eboot.elf at 0x8D8040.
void PS4Device::_TerminateImpl() {
    mSubmitThreadRunning = false;
    mSubmitThread.mThread._Join();
    mSubmitCondition.Destroy();
    _DestroyMainWindow();
    _DestroyContexts();
    _TerminateVideoOutput();
}

// Reconstructed from eboot.elf at 0x8D8040 (inlined into _TerminateImpl).
void PS4Device::_TerminateVideoOutput() {
    sceGnmDeleteEqEvent(mEventQueue, kGnmEventId);
    sceKernelDeleteEqueue(mEventQueue);
    sceVideoOutClose(mVideoOutHandle);
}

// Reconstructed from eboot.elf at 0x8D7B20 (inlined into _InitImpl).
void PS4Device::_WaitForSubmitThread() {
    scePthreadMutexLock(&mSubmitCritSec.mCritSec);
    ++mSubmitCritSec.mEntryCount;
    while (mSubmitToken == 0) {
        mSubmitCondition.Wait();
    }
    --mSubmitCritSec.mEntryCount;
    scePthreadMutexUnlock(&mSubmitCritSec.mCritSec);
}

// Reconstructed from eboot.elf at 0x8D77E0.
int PS4Device::_SubmitDoneThreadEntry(void* device) {
    static_cast<PS4Device*>(device)->_SubmitDoneThread();
    return 0;
}

// Reconstructed from eboot.elf at 0x8D7340.
void PS4Device::_SubmitDoneThread() {
    Lock();
    if (mBeginFramePending) {
        _FlushPendingBeginFrame();
    }
    Unlock();

    scePthreadMutexLock(&mSubmitCritSec.mCritSec);
    mSubmitToken = 1;
    scePthreadMutexUnlock(&mSubmitCritSec.mCritSec);
    mSubmitCondition.Signal();

    PS4SubmitDoneState state;
    state.mLastSubmitCheck = Hmx::Timer::GetCycleCounter();
    std::array<PS4SubmitEvent, 4> events{};
    while (mSubmitThreadRunning) {
        unsigned long eventCount = 0;
        if (!_WaitForSubmitEvents(events.data(), events.size(), eventCount)) {
            _ProcessSubmitTimeout(state);
            continue;
        }

        for (unsigned long index = 0; index < eventCount; ++index) {
            switch (events[index].mType) {
            case PS4SubmitEvent::kFlipComplete:
                _ProcessFlipComplete();
                break;
            case PS4SubmitEvent::kEndOfPipe:
                _ProcessEndOfPipe(state);
                break;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x8D7340 (inlined into _SubmitDoneThread).
bool PS4Device::_WaitForSubmitEvents(
    PS4SubmitEvent* events,
    unsigned long capacity,
    unsigned long& eventCount) {
    constexpr SceKernelUseconds kWaitTimeoutMicroseconds = 1000000;
    std::array<SceKernelEvent, 4> kernelEvents{};
    auto timeout = kWaitTimeoutMicroseconds;
    int kernelEventCount = 0;
    const auto waitCapacity = static_cast<int>(
        std::min<unsigned long>(capacity, kernelEvents.size()));
    const auto result = sceKernelWaitEqueue(
        mEventQueue,
        kernelEvents.data(),
        waitCapacity,
        &kernelEventCount,
        &timeout);
    eventCount = 0;
    if (result != 0) {
        return false;
    }

    for (int index = 0; index < kernelEventCount; ++index) {
        const auto filter = sceKernelGetEventFilter(&kernelEvents[index]);
        if (filter == SCE_KERNEL_EVFILT_VIDEO_OUT) {
            events[eventCount++].mType = PS4SubmitEvent::kFlipComplete;
        } else if (filter == SCE_KERNEL_EVFILT_GNM) {
            events[eventCount++].mType = PS4SubmitEvent::kEndOfPipe;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x8D7340 (inlined into _SubmitDoneThread).
void PS4Device::_ProcessSubmitTimeout(PS4SubmitDoneState& state) {
    scePthreadMutexLock(&mSubmitCritSec.mCritSec);
    ++mSubmitCritSec.mEntryCount;
    state.mLastSubmitCheck = Hmx::Timer::GetCycleCounter();
    sceGnmSubmitDone();
    state.mPendingSubmitTicks = 0;
    --mSubmitCritSec.mEntryCount;
    scePthreadMutexUnlock(&mSubmitCritSec.mCritSec);
}

// Reconstructed from eboot.elf at 0x8D7340 (inlined into _SubmitDoneThread).
void PS4Device::_ProcessFlipComplete() {
    SceVideoOutFlipStatus status{};
    sceVideoOutGetFlipStatus(mVideoOutHandle, &status);

    const auto completedBuffer = static_cast<std::uint64_t>(status.flipArg);
    if (completedBuffer >= kBackBufferCount) {
        return;
    }

    ForEachOutputTexture(*this, [completedBuffer](PS4Texture2D& texture) {
        texture.CompletePendingPresentation(
            static_cast<unsigned long>(completedBuffer));
    });
}

// Reconstructed from eboot.elf at 0x8D7340 (inlined into _SubmitDoneThread).
void PS4Device::_ProcessEndOfPipe(PS4SubmitDoneState& state) {
    scePthreadMutexLock(&mSubmitCritSec.mCritSec);
    ++mSubmitCritSec.mEntryCount;

    bool submitDone = orbis_render_context_frame_submissions_complete(
        Context(), state.mNextBuffer);
    if (!submitDone) {
        const auto now = Hmx::Timer::GetCycleCounter();
        state.mPendingSubmitTicks += now - state.mLastSubmitCheck;
        state.mLastSubmitCheck = now;
        submitDone = static_cast<float>(
            Hmx::Timer::CyclesToMs(state.mPendingSubmitTicks)) >=
            kSubmitDoneTimeoutMilliseconds;
    }

    if (submitDone) {
        state.mLastSubmitCheck = Hmx::Timer::GetCycleCounter();
        sceGnmSubmitDone();
        state.mPendingSubmitTicks = 0;
    }

    ForEachOutputTexture(*this, [&state](PS4Texture2D& texture) {
        texture.AddPendingPresentation(state.mNextBuffer);
    });

    mSubmitToken = 1;
    --mSubmitCritSec.mEntryCount;
    scePthreadMutexUnlock(&mSubmitCritSec.mCritSec);
    mSubmitCondition.Signal();

    const auto rate = mSettings->ActiveVSyncMode();
    if (rate != mFlipRate) {
        mFlipRate = rate;
        sceVideoOutSetFlipRate(mVideoOutHandle, rate == 2 ? 1 : 0);
    }

    sceVideoOutSubmitFlip(
        mVideoOutHandle,
        static_cast<std::int32_t>(state.mNextBuffer),
        VideoFlipMode(rate),
        state.mPreviousBuffer);
    state.mPreviousBuffer = static_cast<long>(state.mNextBuffer);
    state.mNextBuffer = (state.mNextBuffer + 1) % kBackBufferCount;
}
