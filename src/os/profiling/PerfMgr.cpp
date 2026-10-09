#include "os/profiling/PerfMgr.h"

#include "os/memory/MemMgr.h"
#include "os/profiling/PerfTimer.h"

namespace {

// The calling thread's timers, indexed like thePerfMgr's names. In this
// build the thread data lives in the engine's shared per-thread block
// (created at 0x259570), at the offset recorded in 0x19B03A8 (0x5460) by the
// static initializer; the map's build keeps a TLSValue<eastl::vector<
// PerfTimer*>>. Name not in the reference map.
struct PerfThreadTimers {
    PerfThreadTimers() : mThread(scePthreadSelf()) {}
    // Inlined into the thread data's destructor (0x259400).
    ~PerfThreadTimers() {
        thePerfMgr._ThreadTerminate(mThread);
    }

    eastl::vector<PerfTimerBase*> mTimers;
    ScePthread mThread;
};

static_assert(sizeof(PerfThreadTimers) == 40);

thread_local PerfThreadTimers tThreadTimers;  // Name not in the reference map.

// The calling thread's running timers, which its timers push themselves
// onto as they start. It follows the timers in the per-thread block
// (0x5488); the map's build has a TLSValue<PerfTimer**>. Name not in the
// reference map.
thread_local eastl::vector<PerfTimerBase*> tRunningTimers;

// The heap the manager allocates from. Each function looks it up once.
long DebugHeap() {  // Inlined; name not in the reference map.
    return MemFindHeap("debug");
}

}  // namespace

PerfTimerMgr thePerfMgr;

// Reconstructed from eboot.elf at 0x249650.
void PerfTimerMgr::Init(DataArray* config) {
    static long sHeap = DebugHeap();
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mCritSec);
        mEnabled = true;
        DataArray* old = mConfig.mData;
        if (old != config) {
            mConfig.mData = config;
            if (config != nullptr) {
                config->AddRef();
            }
            if (old != nullptr) {
                old->Release();
            }
        }
        mTimerNames.reserve(1024);
        mThreadTimers.reserve(12);
    }
    MemPopHeap();
    _ThreadInit();
}

// Reconstructed from eboot.elf at 0x2498B0.
PerfTimerMgr::~PerfTimerMgr() {
    mEnabled = false;
}

// Reconstructed from eboot.elf at 0x249970.
PerfTimerMgr::ThreadTimers::ThreadTimers(ScePthread thread, eastl::vector<PerfTimerBase*>* timers)
    : mThread(thread), mTimers(timers) {}

// Reconstructed from eboot.elf at 0x249980.
void PerfTimerMgr::Lock() {
    mCritSec.Enter();
}

// Reconstructed from eboot.elf at 0x2499A0.
void PerfTimerMgr::Unlock() {
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x2499B0.
PerfTimer* PerfTimerMgr::GetTimer(Symbol name) {
    return GetTimer(GetTimerIndex(name));
}

// Reconstructed from eboot.elf at 0x2499D0.
unsigned long PerfTimerMgr::GetTimerIndex(Symbol name) {
    ScopedCritSec lock(mCritSec);
    for (unsigned long i = 0; i < mTimerNames.size(); ++i) {
        if (mTimerNames[i] == name) {
            return i;
        }
    }
    static long sHeap = DebugHeap();
    MemPushHeap(sHeap);
    const unsigned long index = mTimerNames.size();
    mTimerNames.push_back(name);
    MemPopHeap();
    return index;
}

// Reconstructed from eboot.elf at 0x249B70.
PerfTimer* PerfTimerMgr::GetTimer(unsigned long index) {
    if (!mEnabled) {
        return nullptr;
    }
    eastl::vector<PerfTimerBase*>& timers = tThreadTimers.mTimers;
    if (timers.capacity() == 0) {
        _ThreadInit();
    }
    if (index >= timers.size() || timers[index] == nullptr) {
        ScopedCritSec lock(mCritSec);
        _AddTimer(mTimerNames[index], nullptr);
    }
    return static_cast<PerfTimer*>(timers[index]);
}

// Reconstructed from eboot.elf at 0x249CA0.
void PerfTimerMgr::SetAllExpanded(bool expanded) {
    ScopedCritSec lock(mCritSec);
    for (ThreadTimers* thread : mThreadTimers) {
        eastl::vector<PerfTimerBase*>& timers = *thread->mTimers;
        for (unsigned long i = 0; i < timers.size(); ++i) {
            if (timers[i] != nullptr) {
                timers[i]->mExpanded = expanded;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x249D40.
void PerfTimerMgr::Poll(bool reset) {
    // The calling thread is read and not used.
    static_cast<void>(scePthreadSelf());
    ScopedCritSec lock(mCritSec);
    for (ThreadTimers* thread : mThreadTimers) {
        eastl::vector<PerfTimerBase*>& timers = *thread->mTimers;
        for (unsigned long i = 0; i < timers.size(); ++i) {
            if (timers[i] != nullptr) {
                static_cast<PerfTimer*>(timers[i])->EndFrame(reset);
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x249E50.
PerfTimer* PerfTimerMgr::_AddTimer(Symbol name, DataArray* config) {
    ScopedCritSec lock(mCritSec);
    if (config != nullptr) {
        bool enabled = true;
        config->FindData(Symbol("enabled"), enabled, false);
        if (!enabled) {
            return nullptr;
        }
    }

    static long sHeap = DebugHeap();
    MemPushHeap(sHeap);
    PerfTimer* timer = new PerfTimer(name, config);
    timer->mRunningTimers = &tRunningTimers;
    const unsigned long index = GetTimerIndex(name);
    eastl::vector<PerfTimerBase*>& timers = tThreadTimers.mTimers;
    if (index >= timers.size()) {
        timers.resize(index + 1);
    }
    timers[index] = timer;
    MemPopHeap();
    return timer;
}

// Reconstructed from eboot.elf at 0x24A0B0.
void PerfTimerMgr::_ThreadInit() {
    ScopedCritSec lock(mCritSec);
    static long sHeap = DebugHeap();
    MemPushHeap(sHeap);

    eastl::vector<PerfTimerBase*>& timers = tThreadTimers.mTimers;
    timers.reserve(1024);
    mThreadTimers.push_back(new ThreadTimers(scePthreadSelf(), &timers));

    // The configuration's first node is its name; each other node is a
    // timer's entry, whose first node is the timer's name.
    if (mConfig.mData != nullptr && (mConfig->Size() & ~1) != 0) {
        for (unsigned long i = 1; i < static_cast<unsigned int>(mConfig->Size()); ++i) {
            DataArray* entry = mConfig->Node(i).mValue.array;
            _AddTimer(entry->Node(0).Sym(entry), entry);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0x24A330.
void PerfTimerMgr::_ThreadTerminate(ScePthread thread) {
    ScopedCritSec lock(mCritSec);
    for (ThreadTimers** entry = mThreadTimers.begin(); entry != mThreadTimers.end(); ++entry) {
        ThreadTimers* timers = *entry;
        if (timers->mThread != thread) {
            continue;
        }
        for (PerfTimerBase* timer : *timers->mTimers) {
            delete timer;
        }
        timers->mTimers->mpEnd = timers->mTimers->mpBegin;
        delete timers;
        mThreadTimers.erase(entry);
        return;
    }
}
