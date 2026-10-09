#pragma once

// Work handed to the main thread (os/CallOnMainThread.o).

namespace Hmx {

// Receives a call on the main thread. MainThreadMessageSender<T> in
// os/PlatformMgr_PS4.o derives from it. Only the slot the queue calls is
// declared.
class MainThreadCallback {
public:
    virtual ~MainThreadCallback() {}
    // Slot 2.
    virtual void OnMainThread(void* context) = 0;
};

// Calls `callback` at once on the main thread, or queues it for the next
// CallOnMainThreadPoll from another thread.
void CallOnMainThread(MainThreadCallback* callback, void* context);  // 0x3902B0
// Runs the queued calls in order and empties the queue. SystemPoll calls
// it.
void CallOnMainThreadPoll();  // 0x390370

}  // namespace Hmx
