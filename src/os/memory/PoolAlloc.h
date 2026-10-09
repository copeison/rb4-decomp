#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"

class FixedString;
class TextStream;

// The small-block pool (os/PoolAlloc.o): 128 size classes of 16 to 2048
// bytes, each a free list refilled 64 items at a time from shared chunks of
// the "main" heap.

// One size class. The map's FixedSizeAlloc is polymorphic and has a
// (unsigned long, unsigned long) constructor; in this build the class has
// no vtable and ChunkAllocator's constructor sets its size.
class FixedSizeAlloc {
public:
    // Inlined into ChunkAllocator's constructor.
    FixedSizeAlloc() : mAllocSize(0), mFreeList(nullptr), mNumAllocs(0), mMaxAllocs(0), mNumItems(0) {}

    // Sets the item size under the lock. Name not in the reference map.
    void SetAllocSize(unsigned long size);  // 0x385A10
    void* Alloc();                          // 0x385A50
    // Links 64 new items from the shared chunk.
    void Refill();                          // 0x385AD0
    void Free(void* allocation);            // 0x385D20

    // Field names are not in the reference map.
    CritSec mCritSec;
    unsigned long mAllocSize;
    void* mFreeList;
    unsigned long mNumAllocs;
    unsigned long mMaxAllocs;
    // The items refilled so far.
    unsigned long mNumItems;
};

static_assert(offsetof(FixedSizeAlloc, mAllocSize) == 0x10);
static_assert(offsetof(FixedSizeAlloc, mNumItems) == 0x30);
static_assert(sizeof(FixedSizeAlloc) == 0x38);

// The size classes, indexed by (size - 1) / 16.
class ChunkAllocator {
public:
    static constexpr int kNumSizes = 128;  // Name not in the reference map.

    ChunkAllocator();  // 0x385D70
    void* Alloc(unsigned long size);              // 0x3853C0
    void Free(void* allocation, unsigned long size);  // 0x3854D0

    FixedSizeAlloc mAllocs[kNumSizes];  // Name not in the reference map.
};

static_assert(sizeof(ChunkAllocator) == 7168);

// The pool, created by its first user. At 0x1A01C48.
extern ChunkAllocator* gChunkAlloc;

// Pool allocation at 0x3852F0 and release at 0x385460. The map has
// PoolAlloc(unsigned long, unsigned long, char const*) and
// PoolFree(unsigned long, void*, char const*); this build's callers pass
// the size and, for allocation, a label, which is not used.
void* PoolAlloc(unsigned long size, const char* name);
void PoolFree(unsigned long size, void* allocation);
// Writes the per-size table.
void PoolReport(TextStream& stream);  // 0x385520
// Appends the pool's one-line summary.
void PoolSummary(FixedString& out);  // 0x3857A0

// Milo's per-class pool allocation overload.
#define POOL_OVERLOAD(class_name)                                   \
    static void* operator new(unsigned long size) {                 \
        return PoolAlloc(size, nullptr);                            \
    }                                                               \
    static void* operator new(unsigned long, void* place) {         \
        return place;                                               \
    }                                                               \
    static void operator delete(void* allocation) {                 \
        PoolFree(sizeof(class_name), allocation);                   \
    }
