#pragma once

#include <_pthread.h>

#include "os/memory/PoolAlloc.h"
#include "utl/threading/Thread.h"

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

// A queued call of a member function with a copy of its argument. It comes
// from the small-block pool with the label "MainFunctionCallerParam" and
// deletes itself once it has run. Every instantiation is emitted in its
// user's object: for example the persistence module's at 0x3AEE80-0x3AEEA0
// (SaveLoadMgr), 0x3AFB80-0x3AFBA0 (ProfileStorageClient) and
// 0x3B0A40-0x3B0A60 (SystemOptionsStorageClient).
template <typename T, typename P>
class MainFunctionCaller : public MainThreadCallback {
public:
    typedef void (T::*MemFunc)(const P& param);

    MainFunctionCaller(T* object, MemFunc func, const P& param)
        : mObject(object), mFunc(func), mParam(param) {}
    ~MainFunctionCaller() override {}

    static void* operator new(unsigned long size) {
        return PoolAlloc(size, "MainFunctionCallerParam");
    }
    static void operator delete(void* allocation) {
        PoolFree(sizeof(MainFunctionCaller), allocation);
    }

    // Slot 2: makes the call, then deletes the caller.
    void OnMainThread(void* context) override {
        static_cast<void>(context);
        (mObject->*mFunc)(mParam);
        delete this;
    }

    // Field names are not in the reference map.
    T* mObject;
    MemFunc mFunc;
    P mParam;
};

// Makes the call at once on the main thread (or before the main thread is
// known), and otherwise queues it there. Inlined into every user in this
// build.
template <typename T, typename P>
void CallMemberFunctionOnMain(T* object, typename MainFunctionCaller<T, P>::MemFunc func, const P& param) {
    if (scePthreadSelf() == Thread::s_MainThreadID || Thread::s_MainThreadID == nullptr) {
        (object->*func)(param);
    } else {
        CallOnMainThread(new MainFunctionCaller<T, P>(object, func, param), nullptr);
    }
}

}  // namespace Hmx
