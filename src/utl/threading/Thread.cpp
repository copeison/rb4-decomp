#include "utl/threading/Thread.h"

#include <cstdio>
#include <cstring>

#include "os/threading/CritSec.h"
#include "utl/text/HmxSnprintf.h"

namespace {

// A registered thread's name. Name not in the reference map.
struct ThreadNameEntry {
    ThreadNameEntry() : mThread(nullptr) {
        mName.mName[0] = '\0';
    }

    ScePthread mThread;
    Thread::ThreadName mName;
};

static_assert(sizeof(ThreadNameEntry) == 40);

constexpr unsigned long kMaxThreadNames = 32;  // Name not in the reference map.

// The registered names, at 0x19E8300, and their lock at 0x19E8800, built by
// Thread.o's static initializer (0x259B80). Names not in the reference map.
ThreadNameEntry gThreadNames[kMaxThreadNames];
CritSec gThreadNamesCritSec;

// The calling thread's name. In this build the thread data lives in the
// engine's shared per-thread block (created at 0x259570), at the offset the
// static initializer records in 0x19B0408 (0x3410); the map's build keeps it
// in a TLSValue<Thread::ThreadName>. Name not in the reference map.
thread_local Thread::ThreadName tThreadName;

int NamedThreadEntry(void* argument) {
    auto& entry = *static_cast<NamedThread::Entry*>(argument);
    Thread::SetCurrentName(entry.mOwner->mThread.Name());
    entry.mResult = entry.mFunc(entry.mContext);
    return entry.mResult;
}

}  // namespace

ScePthread Thread::s_MainThreadID;

unsigned long ThreadMap::gProcessorCount;

namespace {

// The task settings, at 0x19B0410. Only the first group has records.
ThreadMap::TaskGroup gTaskGroups[2] = {
    {8,
     {
         {"thread_call", 1, 0x10000, 1, Thread::kPriorityDefault, 0, {}},
         {"audio_render", 1, 0, 5, static_cast<Thread::ThreadPriority>(699), 0, {}},
         {"audio_2demitter_update", 1, 0, 5, Thread::kPriorityDefault, 0, {}},
         {"mic_reader", 1, 0, 4, static_cast<Thread::ThreadPriority>(699), 0, {}},
         {"stream_reader", 1, 0x10000, 5, Thread::kPriorityDefault, 0, {}},
         {"joypad", 1, 0, 5, static_cast<Thread::ThreadPriority>(699), 0, {}},
         {"job_manager",
          4,
          0,
          0,
          Thread::kPriorityDefault,
          0,
          {{0, 1, Thread::kPriorityDefault, 0},
           {0, 2, Thread::kPriorityDefault, 0},
           {0, 4, Thread::kPriorityDefault, 0}}},
         {nullptr, 0, 0, 0, Thread::kPriorityDefault, 0, {}},
     }},
    {0, {{nullptr, 0, 0, 0, Thread::kPriorityDefault, 0, {}}}},
};

// The settings of unknown tasks, at 0x19B4620.
const ThreadMap::TaskDesc gDefaultTask = {
    "unknown_task", 1, 0x10000, 0, Thread::kPriorityDefault, 0, {}};

}  // namespace

// Reconstructed from eboot.elf at 0x258C60.
unsigned long ThreadMap::GetTaskSettings(const char* name, const TaskDesc** desc) {
    const unsigned long processorCount = gProcessorCount;
    long foundGroup = 0;
    long foundTask = -1;
    unsigned long group = 0;
    unsigned int minProcessors = gTaskGroups[0].mMinProcessors;
    do {
        if (minProcessors == 0) {
            break;
        }
        for (long task = 0; gTaskGroups[group].mTasks[task].mName != nullptr; ++task) {
            if (std::strcmp(gTaskGroups[group].mTasks[task].mName, name) == 0) {
                foundGroup = static_cast<long>(group);
                foundTask = task;
            }
        }
        ++group;
        minProcessors = gTaskGroups[group].mMinProcessors;
    } while (processorCount >= minProcessors);

    const TaskDesc* found =
        foundTask == -1 ? &gDefaultTask : &gTaskGroups[foundGroup].mTasks[foundTask];
    *desc = found;
    return found->mNumThreads;
}

// Reconstructed from eboot.elf at 0x2590B0.
unsigned long ThreadMap::BuildAffinityMask(long processor, unsigned long mask) {
    if (processor != -1) {
        mask |= 1UL << processor;
    }
    if (mask == 0) {
        for (unsigned long i = 0; i < gProcessorCount; ++i) {
            mask |= 1UL << i;
        }
    }
    return mask;
}

// Inlined into 0x259570 and 0x259B80.
Thread::ThreadName::ThreadName() {
    HmxSnprintf(mName, sizeof(mName), "Unknown Thread!");
}

// Reconstructed from eboot.elf at 0x258D60.
const char* Thread::CurrThreadName() {
    return tThreadName.mName;
}

// Reconstructed from eboot.elf at 0x258DE0.
const char* Thread::ThreadIdToName(ScePthread thread) {
    ScopedCritSec lock(gThreadNamesCritSec);
    for (auto& entry : gThreadNames) {
        if (entry.mThread == thread) {
            return entry.mName.mName;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x258E50.
void Thread::SetCurrentName(const char* name) {
    if (s_MainThreadID == nullptr) {
        _InitMainThreadAffinity(&ThreadMap::gProcessorCount);
        s_MainThreadID = scePthreadSelf();
    }
    HmxSnprintf(tThreadName.mName, sizeof(tThreadName.mName), name);

    {
        ScopedCritSec lock(gThreadNamesCritSec);
        for (auto& entry : gThreadNames) {
            if (entry.mThread == nullptr) {
                entry.mThread = scePthreadSelf();
                HmxSnprintf(entry.mName.mName, sizeof(entry.mName.mName), tThreadName.mName);
                break;
            }
        }
    }

    // What remains of a main-thread check in the release build: both paths
    // only touch the thread data.
    if (scePthreadSelf() == s_MainThreadID || s_MainThreadID == nullptr) {
        static_cast<void>(CurrThreadName());
    } else {
        static_cast<void>(CurrThreadName());
    }
}

// Reconstructed from the inlined worker initialization at eboot.elf 0x8D77F0.
void NamedThread::Init(const char* name) {
    mThread.mHandle = nullptr;
    mThread.mProcessor = 0;
    mThread.mPriority = Thread::kPriorityDefault;
    mThread.mStackSize = 0;
    mThread.mAffinityMask = 0;
    std::snprintf(mThread.mName, sizeof(mThread.mName), "%s", name);
    mThread.mEntry = {};
    mThread.mReserved = 0;
    mEntry = {};
}

// Reconstructed from eboot.elf at 0x259210 and 0x2593A0.
void NamedThread::Create(
    int (*func)(void* context),
    void* context,
    const char* name,
    long processor,
    Thread::ThreadPriority priority,
    unsigned int stackSize,
    SceKernelCpumask affinityMask) {
    mEntry.mFunc = func;
    mEntry.mContext = context;
    mEntry.mResult = 0;
    mEntry.mOwner = this;
    mThread.Create(
        NamedThreadEntry,
        &mEntry,
        name,
        processor,
        priority,
        stackSize,
        ThreadMap::BuildAffinityMask(processor, affinityMask));
}
