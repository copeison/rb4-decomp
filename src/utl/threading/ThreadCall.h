#pragma once

// Runs work on the "Hmx ThreadCall" thread and reports the results on the
// polling thread (utl/ThreadCall.o). Calls run one at a time, in order.

// Work for the call thread. ThreadStart runs on the call thread; ThreadDone
// receives its result in ThreadCallPoll. The vtable is at 0x18F0C48; the
// inline destructor pair is emitted at 0x273160 and 0x273170.
class ThreadCallback {
public:
    virtual ~ThreadCallback() {}
    virtual int ThreadStart() = 0;
    virtual void ThreadDone(int result) = 0;
};

// Empties the queue and starts the call thread with the "thread_call" task
// settings.
void ThreadCallInit();  // 0x259D00
// Stops the call thread and waits for it to exit.
void ThreadCallTerminate();  // 0x259E70
// Queues the callback.
void ThreadCall(ThreadCallback* callback);  // 0x259F40
// Reports a finished call and starts the next one.
void ThreadCallPoll();  // 0x259F90
// Whether a call is running.
bool ThreadCallIsBusy();  // 0x25A060
