#include "utl/threading/ThreadCall.h"

#include <unistd.h>

#include "os/threading/Semaphore.h"
#include "utl/threading/Thread.h"

namespace {

// Names in this file are not in the reference map.

enum ThreadCallDataType {
    kThreadCallNone = 0,
    kThreadCallFunc = 1,
    kThreadCallClass = 2,
};

// A queued call.
struct ThreadCallData {
    ThreadCallDataType mType;
    // A function call and its completion; the map's ThreadCall(int (*)(),
    // void (*)(int)) queues these, but that entry point is not in this
    // binary.
    int (*mFunc)();
    void (*mCallback)(int result);
    ThreadCallback* mClass;
    int mResult;
};

static_assert(sizeof(ThreadCallData) == 40);

constexpr int kMaxThreadCalls = 1000;

// The call thread. Thread.o's constructor and destructor are inlined into
// the static initializer at 0x25A070 and its exit handler at 0x9B0.
struct ThreadCallThread : NamedThread {
    ThreadCallThread() {
        Init("Unknown Thread!");
    }
    ~ThreadCallThread() {
        mThread._ForceKillThread();
    }
};

// Signalled once for each call to run, and to quit. At 0x19E8824.
Semaphore gSemaphore;
ThreadCallThread gThread;  // At 0x19E8838.
// The queue, a ring at 0x19E88C0 whose head is the running or finished call.
ThreadCallData gData[kMaxThreadCalls];
int gHead;            // At 0x19F2500.
int gTail;            // At 0x19F2504.
bool gThreadDone;     // At 0x19F2508: the thread has left its loop.
bool gQuit;           // At 0x19F2509.
bool gResultReady;    // At 0x19F250A: the head call has finished.
bool gBusy;           // At 0x19F250B: the head call was started.

// Reconstructed from eboot.elf at 0x259DE0.
int ThreadCallThreadEntry(void* context) {
    static_cast<void>(context);
    for (;;) {
        gSemaphore.Wait();
        if (gQuit) {
            break;
        }

        int result;
        switch (gData[gHead].mType) {
        case kThreadCallClass:
            result = gData[gHead].mClass->ThreadStart();
            break;
        case kThreadCallFunc:
            result = gData[gHead].mFunc();
            break;
        default:
            continue;
        }
        gResultReady = true;
        gData[gHead].mResult = result;
    }
    gThreadDone = true;
    return 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x259D00.
void ThreadCallInit() {
    __builtin_memset(gData, 0, sizeof(gData));
    gHead = 0;
    gTail = 0;
    gSemaphore.Create(0, kMaxThreadCalls);

    long processor = 0;
    Thread::ThreadPriority priority = Thread::kPriorityDefault;
    unsigned int stackSize = 0x10000;
    unsigned long affinityMask = 0;
    const ThreadMap::TaskDesc* task;
    if (ThreadMap::GetTaskSettings("thread_call", &task) != 0) {
        priority = task->mPriority;
        processor = task->mProcessor;
        stackSize = task->mStackSize;
        affinityMask = task->mAffinityMask;
    }
    gThreadDone = false;
    gThread.Create(
        ThreadCallThreadEntry,
        nullptr,
        "Hmx ThreadCall",
        processor,
        priority,
        stackSize,
        affinityMask);
    gThread.mThread.Start();
}

// Reconstructed from eboot.elf at 0x259E70.
void ThreadCallTerminate() {
    if (gThread.mThread.mHandle == nullptr) {
        return;
    }
    gQuit = true;
    gSemaphore.Release();
    while (!gThreadDone) {
        usleep(1000);
    }
    gThread.mThread._Join();
    gSemaphore.Destroy();
}

// Reconstructed from eboot.elf at 0x259F40.
void ThreadCall(ThreadCallback* callback) {
    ThreadCallData& data = gData[gTail];
    data.mType = kThreadCallClass;
    data.mFunc = nullptr;
    data.mCallback = nullptr;
    data.mClass = callback;
    gTail = (gTail + 1) % kMaxThreadCalls;
}

// Reconstructed from eboot.elf at 0x259F90.
void ThreadCallPoll() {
    if (gResultReady) {
        ThreadCallData& data = gData[gHead];
        const ThreadCallDataType type = data.mType;
        if (type != kThreadCallNone) {
            gResultReady = false;
            gBusy = false;
            data.mType = kThreadCallNone;
            gHead = (gHead + 1) % kMaxThreadCalls;
            if (type == kThreadCallClass) {
                data.mClass->ThreadDone(data.mResult);
            } else if (type == kThreadCallFunc) {
                data.mCallback(data.mResult);
            }
        }
    }
    if (!gBusy && gData[gHead].mType != kThreadCallNone) {
        gBusy = true;
        gSemaphore.Release();
    }
}

// Reconstructed from eboot.elf at 0x25A060.
bool ThreadCallIsBusy() {
    return gBusy;
}
