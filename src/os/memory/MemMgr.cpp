#include "os/memory/MemMgr.h"

#include <cstring>

#include "os/debug/Debug.h"
#include "os/memory/MemHeap.h"
#include "os/memory/MemTrack.h"
#include "os/memory/PoolAlloc.h"
#include "os/threading/CritSec.h"
#include "os/threading/TLSValue.h"
#include "utl/containers/Map.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFunc.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// A thread's heap stack: the heaps it pushed and its temporary-allocation
// depth. Field names are not in the reference map.
struct MemHeapStack {
    // The TLSValue's factory zeroes only the depths.
    MemHeapStack() : mDepth(0), mTempDepth(0) {}

    long mHeaps[kMaxHeaps];
    int mDepth;
    // Raised by MemAllocTemp and MemPushTemp; while it is set, allocations
    // use the kLastFit strategy of a heap that allows them.
    int mTempDepth;
};

static_assert(sizeof(MemHeapStack) == 0x88);

// The calling thread's heap stack, created with the lock by the first
// MemAlloc. At 0x1A01B88; the map's tMemStack.
TLSValue<MemHeapStack>* tMemStack;

// Locks gMemLock when it exists. Name not in the reference map; the binary
// inlines it everywhere.
class MemLock {
public:
    MemLock() : mLock(gMemLock) {
        if (mLock != nullptr) {
            mLock->Enter();
        }
    }
    ~MemLock() {
        if (mLock != nullptr) {
            mLock->Exit();
        }
    }

private:
    CritSec* mLock;
};

// The "mem" block's check_consistency value; not otherwise used. At
// 0x1A01B98. Name not in the reference map.
int gCheckConsistency;
// Set by MemInit. At 0x1A01B9C. Name not in the reference map.
bool gMemInitialized;
// The heaps' allocation count and allocated bytes, and the free count. At
// 0x1A01BA0, 0x1A01BA8 and 0x1A01BB0. Names not in the reference map.
int gNumAllocs;
unsigned long gBytesAllocated;
int gNumFrees;
// Set by SetMemoryFailure. At 0x1A01BB4. Name not in the reference map.
bool gMemoryFailure;
// The free bytes MemDelta last printed, at 0x19BDA88. Name not in the
// reference map.
unsigned long sLastDeltaFree = static_cast<unsigned long>(-1);
// The byte units PrintBytes scales to, at 0x19BDA60. Name not in the
// reference map.
const char* const kByteUnits[] = {"Bytes", "KB", "MB", "GB"};

// Script handler for mem_print_concise_heap_report. Name not in the
// reference map.
DataNode OnMemPrintConciseHeapReport(DataArray*) {  // 0x37AE30
    MemPrintConciseHeapReport(TheDebug);
    return DataNode(0);
}

}  // namespace

MemHeap gHeaps[kMaxHeaps];
unsigned long gNumHeaps;
int gDefaultHeap = -1;
bool gBatchFreeListInsertions;
bool gInsideMemFunc;
CritSec* gMemLock;
// Sizes by address, at 0x1A01B50; constructed and destroyed but not
// otherwise used in this build. Name not in the reference map, which
// instantiates eastl::map<void*, unsigned long> in MemMgr.o. Weak.
eastl::map<void*, unsigned long> gAllocSizes;

// Reconstructed from eboot.elf at 0x37A550.
const char* PrintBytes(unsigned long bytes) {
    float value = static_cast<float>(bytes);
    int unit = 0;
    while (__builtin_fabsf(value) > 1024.0f) {
        ++unit;
        value *= 1.0f / 1024.0f;
        if (unit > 3) {
            break;
        }
    }
    FormatString format(unit == 0 ? "%g %s" : "%.1f %s");
    format << value << kByteUnits[unit];
    return format.Str();
}

// Reconstructed from eboot.elf at 0x37A6B0.
long GetCurrentHeapNum() {
    MemHeapStack* stack = tMemStack->Peek();
    if (stack != nullptr && stack->mDepth != 0) {
        return stack->mHeaps[stack->mDepth - 1];
    }
    return gDefaultHeap;
}

// Reconstructed from eboot.elf at 0x37A700.
void MemFreeBlockStats(long heap, MemHeapStats& stats) {
    MemLock lock;
    MemHeap& memHeap = gHeaps[heap];
    stats.mSize = (memHeap.mExtraBytes + 4 * memHeap.mSizeWords) << memHeap.mShift;
    memHeap.FreeBlockStats(stats);
}

// Reconstructed from eboot.elf at 0x37A790.
void MemDelta(const char* label, int heap) {
    MemHeapStats stats;
    MemFreeBlockStats(heap, stats);
    if (sLastDeltaFree == static_cast<unsigned long>(-1)) {
        sLastDeltaFree = stats.mFreeBytes;
    }
    TheDebug << label << " size:" << stats.mSize << " lfrag:" << stats.mLeftFrags << " rfrag:"
             << stats.mRightFrags << " largest:" << stats.mBiggestFree << " free:"
             << stats.mFreeBytes << " delta:" << (sLastDeltaFree - stats.mFreeBytes) << "\n";
    sLastDeltaFree = stats.mFreeBytes;
}

// Reconstructed from eboot.elf at 0x37A920.
void MemPushHeap(long heap) {
    MemHeapStack* stack = tMemStack->Get();
    stack->mHeaps[stack->mDepth++] = heap;
}

// Reconstructed from eboot.elf at 0x37A9B0.
void MemPopHeap() {
    --tMemStack->Get()->mDepth;
}

// Reconstructed from eboot.elf at 0x37AA30.
void MemPushTemp(unsigned int& saved, bool enable, bool apply) {
    saved = 0;
    MemHeapStack* stack = tMemStack->Get();
    saved = stack->mTempDepth;
    if (apply) {
        stack->mTempDepth = enable ? stack->mTempDepth + 1 : 0;
    }
}

// Reconstructed from eboot.elf at 0x37AAF0.
void MemPopTemp(const unsigned int& saved) {
    tMemStack->Get()->mTempDepth = saved;
}

// Reconstructed from eboot.elf at 0x37AB70.
void MemInit(DataArray* config) {
    MemLock lock;
    bool disable = false;
    bool enableTracking = false;
    config->FindData(Symbol("check_consistency"), gCheckConsistency, true);
    config->FindData(Symbol("enable_tracking"), enableTracking, true);
    config->FindData(Symbol("disable_mgr"), disable, true);
    int trackHeap = -1;
    config->FindData(Symbol("track_heap"), trackHeap, false);
    int trackedAllocs = -1;
    config->FindData(Symbol("tracked_allocs"), trackedAllocs, false);
    String trackType("");
    config->FindData(Symbol("track_type"), trackType, false);
    bool callstackTracking = true;
    config->FindData(Symbol("callstack_tracking"), callstackTracking, false);
    // The manager cannot be disabled in this build.
    disable = false;
    if (enableTracking) {
        MemTrackInit(MemTrackParams(trackHeap, trackedAllocs, trackType, callstackTracking));
    }
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        gHeaps[i].InitBatchedFreeList();
    }
    gMemInitialized = true;
    DataRegisterFunc(Symbol("mem_print_concise_heap_report"), OnMemPrintConciseHeapReport);
}

// Reconstructed from eboot.elf at 0x37AE60.
void MemTerminate() {}

// Reconstructed from eboot.elf at 0x37AE70.
void* MemAlloc(unsigned long size, const char* name, int align) {
    if (gMemLock == nullptr) {
        gMemLock = new (PlatformAlloc(sizeof(CritSec), 0)) CritSec;
        tMemStack = new (PlatformAlloc(sizeof(TLSValue<MemHeapStack>), 0)) TLSValue<MemHeapStack>;
        ProjectMemPreInit();
        static_cast<void>(scePthreadSelf());
    }
    MemLock lock;
    MemHeapStack* stack = tMemStack->Peek();
    if (gNumHeaps == 0) {
        return PlatformAlloc(size, align);
    }
    const long current =
        stack != nullptr && stack->mDepth != 0 ? stack->mHeaps[stack->mDepth - 1] : gDefaultHeap;
    if (current == -1) {
        return PlatformAlloc(size, align);
    }

    MemHeap* heap;
    bool lastFit;
    if (stack != nullptr) {
        heap = current >= 0 ? &gHeaps[current] : nullptr;
        lastFit = true;
        bool temp = stack->mTempDepth != 0;
        if (!temp && heap != nullptr) {
            lastFit = heap->mStrategy == MemHeap::kLastFit;
            temp = lastFit;
        }
        if (temp && !heap->mAllowTemp) {
            // Temporary allocations go to the nearest heap down the stack
            // that allows them.
            const int depth = stack->mDepth;
            for (int level = depth - 1; level >= 0; --level) {
                const long num = level != 0 ? stack->mHeaps[level - 1] : gDefaultHeap;
                stack->mDepth = level;
                MemHeap* below = num >= 0 ? &gHeaps[num] : nullptr;
                if (below == nullptr || below->mAllowTemp) {
                    break;
                }
            }
            ++stack->mTempDepth;
            void* allocation = MemAlloc(size, name, align);
            --stack->mTempDepth;
            stack->mDepth = depth;
            return allocation;
        }
        if (!temp && heap == nullptr) {
            lastFit = false;
        }
    } else {
        heap = nullptr;
        lastFit = false;
        if (gDefaultHeap >= 0) {
            heap = &gHeaps[gDefaultHeap];
            lastFit = heap->mStrategy == MemHeap::kLastFit;
        }
    }

    const unsigned long words = heap->GetSizeWords(size);
    const int alignWords = heap->GetAlignWords(align);
    const MemHeap::Strategy strategy = heap->mStrategy;
    heap->mStrategy = lastFit ? MemHeap::kLastFit : strategy;
    bool failed = false;
    unsigned long allocated;
    void* allocation = heap->Alloc(words, alignWords, allocated, true, failed);
    if (allocated != 0) {
        ++gNumAllocs;
        gBytesAllocated += 4 * allocated;
    }
    if (allocation == nullptr) {
        allocation = PlatformAlloc(size, align);
        if (allocation == nullptr) {
            StackString<2048> msg;
            FormatString format(
                "Fallback memory allocation failed! Initally attempting to allocate %d from %s "
                "heap");
            format << size << heap->mName;
            msg << format.Str();
            MemPrintOverview(-3, msg);
            allocation = nullptr;
        }
    }
    if (failed) {
        char bytes[32] = {};
        PrintBytes((heap->mExtraBytes + 4 * heap->mSizeWords) << heap->mShift, bytes, sizeof(bytes));
    }
    heap->mStrategy = strategy;
    return allocation;
}

// Reconstructed from eboot.elf at 0x37B350.
void MemPrintOverview(int heap, FixedString& out) {
    if (gNumHeaps == 0) {
        out += "Memory Manager Disabled\n";
        return;
    }
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        if (heap != kNoHeap && heap != static_cast<long>(i)) {
            continue;
        }
        MemHeapStats stats;
        MemFreeBlockStats(i, stats);
        const char* name = MemHeapName(i);
        char size[32] = {};
        char freeBytes[32] = {};
        char biggest[32] = {};
        char leftFrags[32] = {};
        char rightFrags[32] = {};
        char waste[32] = {};
        char leftAllocs[32] = {};
        char rightAllocs[32] = {};
        FormatString format(
            " [%8s] size:%10s free:%10s big:%10s lfrag:%4s rfrag:%4s waste:%10s lalloc:%8s "
            "ralloc:%8s\n");
        format << name << PrintBytes(stats.mSize, size, sizeof(size))
               << PrintBytes(stats.mFreeBytes, freeBytes, sizeof(freeBytes))
               << PrintBytes(stats.mBiggestFree, biggest, sizeof(biggest))
               << PrintIntWithCommas(stats.mLeftFrags, leftFrags, sizeof(leftFrags))
               << PrintIntWithCommas(stats.mRightFrags, rightFrags, sizeof(rightFrags))
               << PrintBytes(stats.mFreeBytes - stats.mBiggestFree, waste, sizeof(waste))
               << PrintIntWithCommas(stats.mLeftAllocs, leftAllocs, sizeof(leftAllocs))
               << PrintIntWithCommas(stats.mRightAllocs, rightAllocs, sizeof(rightAllocs));
        out += format.Str();
    }
    unsigned long used;
    unsigned long free;
    PlatformPhysicalMemoryUsage(used, free);
    char usedBytes[32] = {};
    char freeBytes[32] = {};
    FormatString format(" [physical] used: %s   free: %s\n");
    format << PrintBytes(used, usedBytes, sizeof(usedBytes))
           << PrintBytes(free, freeBytes, sizeof(freeBytes));
    out += format.Str();
    if (heap == kNoHeap) {
        PoolSummary(out);
    }
}

// Reconstructed from eboot.elf at 0x37B6F0.
void* MemAllocTemp(unsigned long size, const char* name, int align) {
    MemHeapStack* stack = tMemStack->Get();
    const int depth = stack->mTempDepth;
    stack->mTempDepth = depth + 1;
    void* allocation = MemAlloc(size, name, align);
    tMemStack->Get()->mTempDepth = depth;
    return allocation;
}

// Reconstructed from eboot.elf at 0x37B800.
void MemFree(void* allocation) {
    if (allocation == nullptr) {
        return;
    }
    ++gNumFrees;
    if (TheDebug.mExiting) {
        return;
    }
    MemLock lock;
    unsigned long words = 0;
    unsigned long i = 0;
    for (; i < gNumHeaps; ++i) {
        if (gHeaps[i].Free(static_cast<int*>(allocation), words, gBatchFreeListInsertions)) {
            gBytesAllocated -= 4 * words;
            break;
        }
    }
    if (i == gNumHeaps) {
        PlatformFree(allocation);
    }
}

// Reconstructed from eboot.elf at 0x37B930.
void* MemRealloc(void* allocation, unsigned long size, const char* name, int align) {
    MemLock lock;
    if (allocation == nullptr) {
        return MemAlloc(size, name, align);
    }
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        unsigned long oldSize = gHeaps[i].AllocSize(static_cast<int*>(allocation));
        if (oldSize != 0) {
            void* moved = MemAlloc(size, name, align);
            if (oldSize > size) {
                oldSize = size;
            }
            memcpy(moved, allocation, oldSize);
            MemFree(allocation);
            return moved;
        }
    }
    void* moved = PlatformRealloc(allocation, size, align, nullptr);
    MemTrackRealloc(allocation, size, (size + 3) & ~3UL, moved);
    return moved;
}

// Reconstructed from eboot.elf at 0x37BA70.
long MemFindHeap(const char* name) {
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        if (gHeaps[i].mName != nullptr && strcmp(gHeaps[i].mName, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Reconstructed from eboot.elf at 0x37BAE0.
long MemTryFindHeap(const char* name) {
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        if (gHeaps[i].mName != nullptr && strcmp(gHeaps[i].mName, name) == 0) {
            return i;
        }
    }
    return kNoHeap;
}

// Reconstructed from eboot.elf at 0x37BB60.
const char* MemHeapName(long heap) {
    if (heap < 0) {
        return "system";
    }
    return gHeaps[heap].mName;
}

// Reconstructed from eboot.elf at 0x37BB90.
bool MemHeapIsTracked(long heap) {
    return heap >= 0 && gHeaps[heap].mTracked;
}

// Reconstructed from eboot.elf at 0x37BBB0.
unsigned long MemHeapSize(unsigned long heap) {
    return 4 * (gHeaps[heap].mSizeWords << gHeaps[heap].mShift);
}

// Reconstructed from eboot.elf at 0x37BBF0.
int MemHeapStrategy(long heap) {
    MemLock lock;
    return gHeaps[heap].mStrategy;
}

// Reconstructed from eboot.elf at 0x37BC60.
void MemSetHeapStrategy(long heap, int strategy) {
    MemLock lock;
    gHeaps[heap].mStrategy = static_cast<MemHeap::Strategy>(strategy);
}

// Reconstructed from eboot.elf at 0x37BDD0.
void MemPrint(long heap, TextStream& stream, bool freeOnly) {
    MemLock lock;
    gHeaps[heap].Print(stream, freeOnly);
}

// Reconstructed from eboot.elf at 0x37BE60.
void MemBeginBatchingFreeListInsertions() {
    MemLock lock;
    gBatchFreeListInsertions = true;
}

// Reconstructed from eboot.elf at 0x37BEB0.
void MemEndBatchingAndProcessFreeListInsertions() {
    MemLock lock;
    gBatchFreeListInsertions = false;
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        gHeaps[i].ProcessBatchedFreeList();
    }
}

// Reconstructed from eboot.elf at 0x37BF40.
void* operator new(unsigned long size) {
    return MemAlloc(size, "new", 0);
}

// Reconstructed from eboot.elf at 0x37BF50.
void operator delete(void* allocation) noexcept {
    MemFree(allocation);
}

// Reconstructed from eboot.elf at 0x37BF60.
void* operator new[](unsigned long size) {
    return MemAlloc(size, "new[]", 0);
}

// Reconstructed from eboot.elf at 0x37BF70.
void operator delete[](void* allocation) noexcept {
    MemFree(allocation);
}

// Reconstructed from eboot.elf at 0x37C020.
void* MemOrPoolAlloc(unsigned long size, const char* name, int align) {
    if (size == 0) {
        return nullptr;
    }
    if (size < 0x81) {
        return PoolAlloc(size, name);
    }
    return MemAlloc(size, name, align);
}

// Reconstructed from eboot.elf at 0x37C040.
void MemOrPoolFree(unsigned long size, void* allocation, const char*) {
    if (allocation == nullptr) {
        return;
    }
    if (size < 0x81) {
        PoolFree(size, allocation);
    } else {
        MemFree(allocation);
    }
}

// Reconstructed from eboot.elf at 0x37C060.
bool IsInMemoryFailure() {
    return gMemoryFailure;
}

// Reconstructed from eboot.elf at 0x37C070.
void SetMemoryFailure() {
    gMemoryFailure = true;
}

// Reconstructed from eboot.elf at 0x37C080.
void MemGetStats(unsigned long& bytes, int& allocs, int& frees) {
    bytes = gBytesAllocated;
    allocs = gNumAllocs;
    frees = gNumFrees;
}

// Reconstructed from eboot.elf at 0x37C0A0.
void MemPrintConciseHeapReport(TextStream& stream) {
    for (unsigned long i = 0; i < gNumHeaps; ++i) {
        const char* name = MemHeapName(i);
        MemHeapStats stats;
        MemFreeBlockStats(i, stats);
        stream << "(" << name << " siz:" << stats.mSize << " fre:" << stats.mFreeBytes << " lfrg:"
               << stats.mLeftFrags << " rfrg:" << stats.mRightFrags << ") ";
    }
    TheDebug << "\n";
}
