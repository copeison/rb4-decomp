#pragma once

#include <cstddef>

#include "render/system/RndDevice.h"
#include "utl/text/MakeString.h"
#include "utl/threading/PollDep.h"

// The two jobs of a draw on a deferred context, defined in a header and
// emitted next to render/RndDefaults.o (0x6C02F0-0x6C05AF). A draw job,
// derived from RndDeferredContextDrawJob, acquires a deferred context and
// draws into it on a worker; the RndExecDeferredContextJob after it then
// executes the context. RndSceneDrawJobs (0x430610) and the texture
// renderers' groups (RndTexRendererGroup) build them. Names not in the
// reference map; they follow the job's report name.

// Executes the deferred context a draw job acquired (vtable 0x1937850).
class RndExecDeferredContextJob : public PollDepBase {
public:
    // Emitted at 0x6C0330.
    RndExecDeferredContextJob() : mContextIndex(-1UL), mAcquired(false) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x6C0550, 0x6C0560.
    ~RndExecDeferredContextJob() override {}

    // Slot 3 at 0x6C0370: executes an acquired context and forgets it.
    void ThreadPoll(const int& thread) override {
        static_cast<void>(thread);
        if (mAcquired) {
            gRndDevice->ExecuteDeferredContext(mContextIndex);
        }
        mContextIndex = -1UL;
        mAcquired = false;
    }
    // Slot 5 at 0x6C03C0.
    const char* GetPollName() const override {
        return MakeString("draw exec deferred context %d", mContextIndex);
    }

    // The deferred context's index, or -1.
    unsigned long mContextIndex;
    // Whether the draw job acquired the context.
    bool mAcquired;
};

static_assert(offsetof(RndExecDeferredContextJob, mContextIndex) == 144);
static_assert(sizeof(RndExecDeferredContextJob) == 160);

// The base of the draw jobs (vtable 0x19377F0): its execute job waits for
// it. Slot 3 stays pure.
class RndDeferredContextDrawJob : public PollDepBase {
public:
    // Emitted at 0x6C02F0.
    explicit RndDeferredContextDrawJob(RndExecDeferredContextJob* execJob)
        : mExecJob(execJob) {
        execJob->AddPollDep(this);
    }
    // Slots 0-1: 0x6C0580, 0x6C0590.
    ~RndDeferredContextDrawJob() override {}

    RndExecDeferredContextJob* mExecJob;
};

static_assert(offsetof(RndDeferredContextDrawJob, mExecJob) == 144);
static_assert(sizeof(RndDeferredContextDrawJob) == 152);
