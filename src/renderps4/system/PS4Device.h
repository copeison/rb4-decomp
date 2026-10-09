#pragma once

#include <cstddef>
#include <_pthread.h>
#include <gnm/buffer.h>
#include <kernel/equeue.h>

#include "os/threading/Condition.h"
#include "render/system/RndDevice.h"
#include "utl/threading/Thread.h"

class PS4Context;
struct PS4SubmitDoneState;

// Event seen by the submit-done thread. Name not in the reference map.
struct PS4SubmitEvent {
    enum Type {
        kFlipComplete,
        kEndOfPipe,
    };
    Type mType;
};

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
        FixedVector<RndWindow*, 6>& windows,
        bool offscreen) override;                          // slot 7 at 0x8D8300
    int _GetGpuBlockingBehaviorImpl() const override;      // slot 13 at 0x8D8590
    int _GetDeviceStatusImpl() override;                   // slot 14 at 0x8D83E0

    void _InitDefaultVertexBuffers();                      // 0x8D7DB0
    void _InitIdentityInstanceBuffers();                   // 0x8D7EB0
    // Waits until every submitted frame has completed.
    void WaitForIdle();                                    // 0x8D8140
    // Retires a GPU allocation until the current frame completes.
    void DeferredDelete(void* allocation);                 // 0x8D83F0
    // Frees allocations retired at least two frames ago. Name not in the
    // reference map.
    void _ReleaseRetiredAllocations();                     // 0x8D8200

    // Opens video output and the flip/end-of-pipe event queue. Inlined into
    // _InitImpl at 0x8D7B20 in this binary.
    void _InitVideoOutput();
    // Unregisters the end-of-pipe event and closes the event queue and video
    // output. Inlined into _TerminateImpl at 0x8D8040 in this binary.
    void _TerminateVideoOutput();

    // Submit-done thread. Names not in the reference map.
    static int _SubmitDoneThreadEntry(void* device);       // 0x8D77E0
    void _SubmitDoneThread();                              // 0x8D7340
    // Blocks until the submit-done thread publishes its first token. Name not
    // in the reference map; inlined into _InitImpl.
    void _WaitForSubmitThread();
    // Waits up to one second on the event queue and reports the flip and
    // end-of-pipe events in order. Name not in the reference map; inlined
    // into _SubmitDoneThread.
    bool _WaitForSubmitEvents(
        PS4SubmitEvent* events,
        unsigned long capacity,
        unsigned long& eventCount);
    // Names not in the reference map; inlined into _SubmitDoneThread.
    void _ProcessSubmitTimeout(PS4SubmitDoneState& state);
    void _ProcessFlipComplete();
    void _ProcessEndOfPipe(PS4SubmitDoneState& state);

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
    sce::Gnm::Buffer mDefaultVertexDescs[8];
    void* mDefaultVertexBuffer;
    sce::Gnm::Buffer mIdentityInstanceDescs[9];
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
