#pragma once

#include <cstddef>
#include <_pthread.h>

// Platform thread. Its methods are in the Thread_PS4 object.
class Thread {
public:
    enum ThreadPriority : int {
        // Name not in the reference map.
        kPriorityDefault = 700,
    };

    // Entry trampoline arguments. Name not in the reference map.
    struct Entry {
        int (*mFunc)(void* context);
        void* mContext;
        int mResult;
    };

    // Reconstructed from eboot.elf at 0x25C3E0. The map's signature lacks the
    // affinity mask added in this build.
    void Create(
        int (*func)(void* context),
        void* context,
        const char* name,
        long processor,
        ThreadPriority priority,
        unsigned int stackSize,
        SceKernelCpumask affinityMask);
    void Start();
    int _Join();
    void _ForceKillThread();

    // The stored name is reached directly by the wrapper entry.
    const char* Name() const {  // Name not in the reference map.
        return mName;
    }

    // Records the calling thread's name for engine diagnostics. Name not in
    // the reference map.
    static void SetCurrentName(const char* name);
    // The name the thread registered, or null when it registered none.
    static const char* ThreadIdToName(ScePthread thread);  // 0x258DE0

    // The main thread, recorded when it first registers its name. At
    // 0x19E8810.
    static ScePthread s_MainThreadID;

    // Field names are not in the reference map.
    ScePthread mHandle;
    long mProcessor;
    ThreadPriority mPriority;
    unsigned int mStackSize;
    SceKernelCpumask mAffinityMask;
    char mName[32];
    Entry mEntry;
    // Cleared by Create (0x25C3E0); nothing reads it.
    unsigned long mReserved;
};

static_assert(sizeof(Thread::Entry) == 24);
static_assert(offsetof(Thread, mPriority) == 16);
static_assert(offsetof(Thread, mStackSize) == 20);
static_assert(offsetof(Thread, mAffinityMask) == 24);
static_assert(offsetof(Thread, mName) == 32);
static_assert(offsetof(Thread, mEntry) == 64);
static_assert(sizeof(Thread) == 96);

// Thread whose entry registers its name before running the callback. Name not
// in the reference map.
class NamedThread {
public:
    struct Entry {
        int (*mFunc)(void* context);
        void* mContext;
        int mResult;
        NamedThread* mOwner;
    };

    // Inlined default state, recovered from the worker initialization at
    // 0x8D77F0.
    void Init(const char* name);
    // Reconstructed from eboot.elf at 0x259210, which tail-calls the shared
    // body at 0x2593A0.
    void Create(
        int (*func)(void* context),
        void* context,
        const char* name,
        long processor,
        Thread::ThreadPriority priority,
        unsigned int stackSize,
        SceKernelCpumask affinityMask);

    // Never read or written by NamedThread's code; the owner pointer in
    // mEntry reaches the thread name at +40. Name not in the reference map.
    unsigned char mReserved[8];
    Thread mThread;              // Name not in the reference map.
    Entry mEntry;                // Name not in the reference map.
};

static_assert(sizeof(NamedThread::Entry) == 32);
static_assert(offsetof(NamedThread, mThread) == 8);
static_assert(offsetof(NamedThread, mEntry) == 104);
static_assert(sizeof(NamedThread) == 136);

namespace ThreadMap {

constexpr long kProcessorCount = 6;  // Name not in the reference map.

// View of the fields consumed from the engine's 0x210-byte task record,
// whose table starts at 0x19B0410. Field names are not in the reference map.
struct TaskDesc {
    const char* mName;  // Such as "audio_render".
    // Nonzero in every record; the lookup at 0x258C60 returns it, and the
    // "thread_call" setup (0x259D00) uses the record only when it is set.
    unsigned long mEnabled;
    unsigned int mStackSize;
    unsigned char mPadding20[4];  // Zero in every record; never read.
    long mProcessor;
    Thread::ThreadPriority mPriority;
    unsigned char mPadding36[4];  // Zero in every record; never read.
    unsigned long mAffinityMask;
};

static_assert(offsetof(TaskDesc, mEnabled) == 8);
static_assert(offsetof(TaskDesc, mStackSize) == 16);
static_assert(offsetof(TaskDesc, mProcessor) == 24);
static_assert(offsetof(TaskDesc, mPriority) == 32);
static_assert(offsetof(TaskDesc, mAffinityMask) == 40);

// Named task lookup. The map has GetTaskSettings(char const*,
// TaskDesc const**); this build returns the record.
const TaskDesc* GetTaskSettings(const char* name);

// Reconstructed from eboot.elf at 0x2590B0. Name not in the reference map.
unsigned long BuildAffinityMask(long processor, unsigned long mask);

}  // namespace ThreadMap
