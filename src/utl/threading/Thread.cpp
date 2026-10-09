#include "utl/threading/Thread.h"

#include <cstdio>

namespace {

int NamedThreadEntry(void* argument) {
    auto& entry = *static_cast<NamedThread::Entry*>(argument);
    Thread::SetCurrentName(entry.mOwner->mThread.Name());
    entry.mResult = entry.mFunc(entry.mContext);
    return entry.mResult;
}

}  // namespace

// Reconstructed from eboot.elf at 0x2590B0.
unsigned long ThreadMap::BuildAffinityMask(long processor, unsigned long mask) {
    if (processor != -1) {
        mask |= 1UL << processor;
    }
    if (mask != 0) {
        return mask;
    }
    return (1UL << kProcessorCount) - 1;
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
    mThread.mUnknown88 = 0;
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
