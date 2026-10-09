#pragma once

#include "utl/threading/PollDep.h"

// A poll job whose work is two virtuals of its own: ThreadPoll runs _DoPoll
// and PostPoll runs _DoPostPoll. The map has only inline members (emitted
// in render/RndBasicCuller.o); this build emits the bodies in each object
// that defines a subclass, for example at 0x412040-0x412060 for
// RndSceneCom::PollSplitJob (vtable 0x18FFBC8, 12 slots).
class ThreadedJob : public PollDepBase {
public:
    // Slots 3-4 (0x412040, 0x412050 in RndSceneCom.o).
    void ThreadPoll(const int& thread) override {
        static_cast<void>(thread);
        _DoPoll();
    }
    void PostPoll() override {
        _DoPostPoll();
    }
    // Slot 5 (0x412060 in RndSceneCom.o).
    const char* GetPollName() const override {
        return "(threaded job)";
    }
    // Slots 10-11: the job's work and its post-poll work. The map's
    // _DoPoll() and _DoPostPoll().
    virtual void _DoPoll() = 0;
    virtual void _DoPostPoll() = 0;
};
