#include "utl/threading/Thread.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <sys/sched.h>

namespace {

constexpr unsigned int kMinimumStackSize = 0x20000;

void* ThreadEntry(void* argument) {
    auto& entry = *static_cast<Thread::Entry*>(argument);
    entry.mResult = entry.mFunc(entry.mContext);
    return reinterpret_cast<void*>(static_cast<std::intptr_t>(entry.mResult));
}

}  // namespace

// Reconstructed from eboot.elf at 0x25C3E0.
void Thread::Create(
    int (*func)(void* context),
    void* context,
    const char* name,
    long processor,
    ThreadPriority priority,
    unsigned int stackSize,
    SceKernelCpumask affinityMask) {
    mEntry.mFunc = func;
    mEntry.mContext = context;
    mEntry.mResult = 0;
    mReserved = 0;
    mProcessor = processor;
    mPriority = priority;
    mStackSize = std::max(stackSize, kMinimumStackSize);
    mAffinityMask = affinityMask;
    std::snprintf(mName, sizeof(mName), "%s", name);
}

// Reconstructed from eboot.elf at 0x25C430.
void Thread::Start() {
    ScePthreadAttr attributes;
    scePthreadAttrInit(&attributes);
    scePthreadAttrSetdetachstate(&attributes, 0);
    if (mStackSize != 0) {
        scePthreadAttrSetstacksize(&attributes, mStackSize);
    }

    SceKernelSchedParam scheduling{mPriority};
    scePthreadAttrSetinheritsched(&attributes, 0);
    scePthreadAttrSetschedpolicy(&attributes, SCHED_RR);
    scePthreadAttrSetschedparam(&attributes, &scheduling);
    scePthreadAttrSetaffinity(&attributes, mAffinityMask);
    scePthreadCreate(&mHandle, &attributes, ThreadEntry, &mEntry, mName);
    scePthreadRename(mHandle, mName);
    scePthreadAttrDestroy(&attributes);
}

// Reconstructed from eboot.elf at 0x25C530.
int Thread::_Join() {
    if (mHandle == nullptr) {
        return 0;
    }

    void* result = nullptr;
    scePthreadJoin(mHandle, &result);
    mHandle = nullptr;
    return mEntry.mResult;
}

// Reconstructed from eboot.elf at 0x25C3C0.
void Thread::_ForceKillThread() {
    if (mHandle != nullptr) {
        scePthreadCancel(mHandle);
    }
}
