#pragma once

#include <cstddef>
#include <_pthread.h>
#include <kernel/equeue.h>

#include "os/threading/Condition.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"
#include "render/system/RndDevice.h"
#include "utl/threading/Thread.h"

class PS4Context;

// Node of the deferred-delete list. Name not in the reference map.
struct PS4DeferredDelete {
    PS4DeferredDelete* mNext;
    PS4DeferredDelete* mPrev;
    void* mAllocation;
    unsigned long mFrame;  // Frame count when the allocation was retired.
};

// Circular list of deferred deletes; the anchor is the list itself. Name not
// in the reference map.
struct PS4DeferredDeleteList {
    PS4DeferredDelete* mNext;
    PS4DeferredDelete* mPrev;
    unsigned long mSize;
    HmxAllocator::allocator mAllocator;
};

// PS4 render device: video output, the flip and end-of-pipe event queue, the
// submit-done thread, the default vertex and identity-instance buffers, and
// GPU allocations retired until their frame completes. The vtable is at
// 0x195ED70. Its fields start in RndDevice's tail padding at 3804.
class PS4Device : public RndDevice {
public:
    PS4Device();             // 0x8D77F0
    ~PS4Device() override;   // 0x8D79B0, 0x8D7B00

    void _ProcessDeferredDeletion() override;              // slot 2 at 0x8D84B0
    void _InitImpl(const RndInitParams* params) override;  // slot 3 at 0x8D7B20
    void _PostInitImpl() override;                         // slot 4 at 0x8D8580
    void _TerminateImpl() override;                        // slot 5 at 0x8D8040
    // Waits for the GPU and frees retired allocations before the frame.
    void _BeginFrameImpl(bool offscreen) override;         // slot 6 at 0x8D8100
    void _EndFrameImpl(
        FixedVector<rb4::RenderFrameOwner*, 6>& windows,
        bool offscreen) override;                          // slot 7 at 0x8D8300
    int _GetGpuBlockingBehaviorImpl() const override;      // slot 13 at 0x8D8590
    int _UnknownSlot14Impl() override;                     // slot 14 at 0x8D83E0

    void _InitDefaultVertexBuffers();                      // 0x8D7DB0
    void _InitIdentityInstanceBuffers();                   // 0x8D7EB0
    // Waits until every submitted frame has completed.
    void WaitForIdle();                                    // 0x8D8140
    // Retires a GPU allocation until the current frame completes.
    void DeferredDelete(void* allocation);                 // 0x8D83F0
    // Frees allocations retired at least two frames ago. Name not in the
    // reference map.
    void _ReleaseRetiredAllocations();                     // 0x8D8200

    // The immediate context, which _InitImpl creates as a PS4Context.
    PS4Context& Context() {
        return *reinterpret_cast<PS4Context*>(mImmediateContext);
    }

    // Field names are not in the reference map.
    int mVideoOutHandle;
    SceKernelEqueue mEventQueue;
    Condition mSubmitCondition;
    unsigned long mSubmitToken;
    bool mSubmitThreadRunning;
    rb4::OrbisBufferDescriptor mDefaultVertexDescs[8];
    void* mDefaultVertexBuffer;
    rb4::OrbisBufferDescriptor mIdentityInstanceDescs[9];
    void* mIdentityInstanceBuffer;
    NamedThread mSubmitThread;
    CritSec mSubmitCritSec;
    CritSec mDeferredDeleteCritSec;
    PS4DeferredDeleteList mDeferredDeletes;
    int mFlipRate;
};

static_assert(offsetof(PS4Device, mVideoOutHandle) == 3804);
static_assert(offsetof(PS4Device, mEventQueue) == 3808);
static_assert(offsetof(PS4Device, mSubmitCondition) == 3816);
static_assert(offsetof(PS4Device, mSubmitToken) == 3840);
static_assert(offsetof(PS4Device, mSubmitThreadRunning) == 3848);
static_assert(offsetof(PS4Device, mDefaultVertexDescs) == 3852);
static_assert(offsetof(PS4Device, mDefaultVertexBuffer) == 3984);
static_assert(offsetof(PS4Device, mIdentityInstanceDescs) == 3992);
static_assert(offsetof(PS4Device, mIdentityInstanceBuffer) == 4136);
static_assert(offsetof(PS4Device, mSubmitThread) == 4144);
static_assert(offsetof(PS4Device, mSubmitCritSec) == 4280);
static_assert(offsetof(PS4Device, mDeferredDeleteCritSec) == 4296);
static_assert(offsetof(PS4Device, mDeferredDeletes) == 4312);
static_assert(offsetof(PS4Device, mFlipRate) == 4344);
static_assert(sizeof(PS4Device) == 4352);

// The active PS4 device. Name not in the reference map.
extern PS4Device* gPS4Device;
