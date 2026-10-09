#include "os/memory/PoolAlloc.h"

#include <new>

#include "os/debug/Debug.h"
#include "os/memory/MemMgr.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

ChunkAllocator* gChunkAlloc;

namespace {

// The shared chunk the size classes refill from: its unused range, whether
// a refill is taking from it, and how many chunks were allocated. At
// 0x1A01C80, 0x1A01C88, 0x1A01C78 and 0x1A01CA0. Names not in the reference
// map.
char* gPoolChunkCur;
char* gPoolChunkEnd;
bool gPoolChunkInUse;
unsigned long gNumPoolChunks;
// The bytes of all chunks, and the chunk ends left unused when a new chunk
// was started. At 0x1A01C50 and 0x1A01C58. Names not in the reference map.
unsigned long gPoolTotalBytes;
unsigned long gPoolOverheadBytes;

// The pool, created on first use. Name not in the reference map; the binary
// inlines it into every user.
ChunkAllocator* GetChunkAlloc() {
    if (gChunkAlloc == nullptr) {
        gChunkAlloc = new (MemAlloc(sizeof(ChunkAllocator), "PoolAllocator", 0)) ChunkAllocator;
    }
    return gChunkAlloc;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3852F0.
void* PoolAlloc(unsigned long size, const char*) {
    return GetChunkAlloc()->Alloc(size);
}

// Reconstructed from eboot.elf at 0x3853C0.
void* ChunkAllocator::Alloc(unsigned long size) {
    return mAllocs[(size - 1) >> 4].Alloc();
}

// Reconstructed from eboot.elf at 0x385460.
void PoolFree(unsigned long size, void* allocation) {
    if (allocation != nullptr && !TheDebug.mExiting) {
        gChunkAlloc->Free(allocation, size);
    }
}

// Reconstructed from eboot.elf at 0x3854D0.
void ChunkAllocator::Free(void* allocation, unsigned long size) {
    FixedSizeAlloc& alloc = mAllocs[(size - 1) >> 4];
    ScopedCritSec lock(alloc.mCritSec);
    *static_cast<void**>(allocation) = alloc.mFreeList;
    alloc.mFreeList = allocation;
    --alloc.mNumAllocs;
}

// Reconstructed from eboot.elf at 0x385520.
void PoolReport(TextStream& stream) {
    ChunkAllocator* chunkAlloc = GetChunkAlloc();
    {
        FormatString format("\n*** POOL REPORT (Total Heap Usage: %d Overhead: %d)***\n");
        format << gPoolTotalBytes << gPoolOverheadBytes;
        stream << format.Str();
    }
    {
        FormatString format("   ItemSize    NumItems   MaxItems  Capacity  Unused\n");
        stream << format.Str();
    }
    unsigned long total = 0;
    unsigned long unused = 0;
    for (FixedSizeAlloc& alloc : chunkAlloc->mAllocs) {
        const unsigned long size = alloc.mAllocSize;
        const unsigned long num = alloc.mNumAllocs;
        const unsigned long free = size * (alloc.mNumItems - num);
        FormatString format("   %8d  %8d  %8d  %8d  %8d\n");
        format << size << num << alloc.mMaxAllocs << alloc.mNumItems << free;
        stream << format.Str();
        unused += free;
        total += size * num;
    }
    {
        FormatString format("                      Total Memory Usage = %8d\n");
        format << total;
        stream << format.Str();
    }
    {
        FormatString format("                            Total Unused = %8d\n");
        format << unused;
        stream << format.Str();
    }
}

// Reconstructed from eboot.elf at 0x3857A0.
void PoolSummary(FixedString& out) {
    ChunkAllocator* chunkAlloc = GetChunkAlloc();
    unsigned long items = 0;
    unsigned long maxItems = 0;
    unsigned long capacity = 0;
    unsigned long unused = 0;
    unsigned long usage = 0;
    for (FixedSizeAlloc& alloc : chunkAlloc->mAllocs) {
        const unsigned long size = alloc.mAllocSize;
        maxItems += alloc.mMaxAllocs;
        items += alloc.mNumAllocs;
        usage += size * alloc.mNumAllocs;
        capacity += size * alloc.mNumItems;
        unused += size * (alloc.mNumItems - alloc.mNumAllocs);
    }
    char itemsText[32] = {};
    char maxItemsText[32] = {};
    char capacityText[32] = {};
    char usageText[32] = {};
    char unusedText[32] = {};
    char overheadText[32] = {};
    FormatString format(
        " [  pooled] items: %s  max items: %s  capacity: %s  usage: %s  unused: %s  overhead: %s\n");
    format << PrintIntWithCommas(items, itemsText, sizeof(itemsText))
           << PrintIntWithCommas(maxItems, maxItemsText, sizeof(maxItemsText))
           << PrintBytes(capacity, capacityText, sizeof(capacityText))
           << PrintBytes(usage, usageText, sizeof(usageText))
           << PrintBytes(unused, unusedText, sizeof(unusedText))
           << PrintBytes(gPoolTotalBytes - capacity, overheadText, sizeof(overheadText));
    out += format.Str();
}

// Reconstructed from eboot.elf at 0x385A10.
void FixedSizeAlloc::SetAllocSize(unsigned long size) {
    ScopedCritSec lock(mCritSec);
    mAllocSize = size;
}

// Reconstructed from eboot.elf at 0x385A50.
void* FixedSizeAlloc::Alloc() {
    ScopedCritSec lock(mCritSec);
    if (mFreeList == nullptr) {
        Refill();
    }
    void* allocation = mFreeList;
    mFreeList = *static_cast<void**>(allocation);
    ++mNumAllocs;
    mMaxAllocs = mMaxAllocs >= mNumAllocs ? mMaxAllocs : mNumAllocs;
    return allocation;
}

// Reconstructed from eboot.elf at 0x385AD0.
void FixedSizeAlloc::Refill() {
    ScopedCritSec lock(mCritSec);
    const unsigned long bytes = mAllocSize << 6;
    char* items;
    {
        static CritSec sChunkCritSec;
        ScopedCritSec chunkLock(sChunkCritSec);
        gPoolChunkInUse = true;
        if (gPoolChunkCur + bytes > gPoolChunkEnd) {
            // The heap's name includes the quotes, so it is never found and
            // the chunks come from the default heap.
            static long sMainHeap = MemFindHeap("\"main\"");
            MemPushHeap(sMainHeap);
            const unsigned long chunkSize =
                gNumPoolChunks != 0 ? 0x100000UL << (gNumPoolChunks - 1) : 0x1000000UL;
            gPoolTotalBytes += chunkSize;
            gPoolOverheadBytes += gPoolChunkEnd - gPoolChunkCur;
            auto* chunk = static_cast<char*>(MemAlloc(chunkSize, "PoolChunk", 16));
            ++gNumPoolChunks;
            gPoolChunkEnd = chunk + chunkSize;
            gPoolChunkCur = chunk + 16;
            MemPopHeap();
        }
        items = gPoolChunkCur;
        gPoolChunkInUse = false;
        gPoolChunkCur = items + bytes;
    }
    char* item = items;
    char* last = items + bytes - mAllocSize;
    while (item < last) {
        *reinterpret_cast<char**>(item) = item + mAllocSize;
        item += mAllocSize;
    }
    *reinterpret_cast<char**>(item) = nullptr;
    mNumItems += 64;
    mFreeList = items;
}

// Reconstructed from eboot.elf at 0x385D20.
void FixedSizeAlloc::Free(void* allocation) {
    ScopedCritSec lock(mCritSec);
    *static_cast<void**>(allocation) = mFreeList;
    mFreeList = allocation;
    --mNumAllocs;
}

// Reconstructed from eboot.elf at 0x385D70.
ChunkAllocator::ChunkAllocator() {
    for (int i = 0; i < kNumSizes; ++i) {
        mAllocs[i].SetAllocSize(16 * (i + 1));
    }
}
