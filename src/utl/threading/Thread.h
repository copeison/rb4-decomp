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

    // A thread's name. The constructor, inlined into the thread-data
    // creator (0x259570) and Thread.o's static initializer (0x259B80),
    // writes "Unknown Thread!".
    struct ThreadName {
        ThreadName();

        char mName[32];  // Name not in the reference map.
    };

    // Records the calling thread's name in its thread data and in the first
    // free entry of the name table. The first call, made on the main thread,
    // also records the main thread and pins it. Name not in the reference
    // map.
    static void SetCurrentName(const char* name);  // 0x258E50
    // The calling thread's name.
    static const char* CurrThreadName();  // 0x258D60
    // The name the thread registered, or null when it registered none.
    static const char* ThreadIdToName(ScePthread thread);  // 0x258DE0
    // Sets the processor count to six and pins the calling (main) thread
    // to processor 3. Name not in the reference map.
    static void _InitMainThreadAffinity(unsigned long* processorCount);  // 0x25C3A0

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

// The number of processors the engine schedules on, set to six when the
// main thread registers. At 0x19E8818. Name not in the reference map.
extern unsigned long gProcessorCount;

// One thread's settings in a task record. Name not in the reference map.
struct ThreadDesc {
    unsigned int mStackSize;
    long mProcessor;
    Thread::ThreadPriority mPriority;
    unsigned long mAffinityMask;
};

static_assert(sizeof(ThreadDesc) == 32);

// The settings of a named task's threads, 0x210 bytes. Single-threaded tasks
// use the first thread's settings, which are spelled out so that callers
// read them directly. Field names are not in the reference map.
struct TaskDesc {
    const char* mName;  // Such as "audio_render".
    // The number of threads: four for "job_manager", one for the others.
    unsigned long mNumThreads;
    unsigned int mStackSize;
    long mProcessor;
    Thread::ThreadPriority mPriority;
    unsigned long mAffinityMask;
    ThreadDesc mMoreThreads[15];
};

static_assert(offsetof(TaskDesc, mNumThreads) == 8);
static_assert(offsetof(TaskDesc, mStackSize) == 16);
static_assert(offsetof(TaskDesc, mProcessor) == 24);
static_assert(offsetof(TaskDesc, mPriority) == 32);
static_assert(offsetof(TaskDesc, mAffinityMask) == 40);
static_assert(sizeof(TaskDesc) == 0x210);

// Sixteen task records used once the processor count reaches the minimum;
// a null name ends the records. Name not in the reference map.
struct TaskGroup {
    unsigned int mMinProcessors;  // Zero ends the groups.
    TaskDesc mTasks[16];
};

static_assert(sizeof(TaskGroup) == 8456);

// The task settings by name. The table at 0x19B0410 holds groups of
// sixteen records, each group used when the processor count reaches its
// minimum; a later match overrides an earlier one, and unknown names get
// the default record at 0x19B4620. Stores the record in `desc` and returns
// its thread count.
unsigned long GetTaskSettings(const char* name, const TaskDesc** desc);  // 0x258C60

// The record alone, for callers that ignore the result. Inlined into its
// callers. Name not in the reference map.
inline const TaskDesc* GetTaskSettings(const char* name) {
    const TaskDesc* desc;
    GetTaskSettings(name, &desc);
    return desc;
}

// Reconstructed from eboot.elf at 0x2590B0. Name not in the reference map.
unsigned long BuildAffinityMask(long processor, unsigned long mask);

}  // namespace ThreadMap
