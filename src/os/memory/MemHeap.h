#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

class TextStream;

// A free run of words in a heap's memory, linked into the heap's free list
// in address order. The minimum block is these six words.
struct FreeBlock {
    // Absorbs `next` when it starts where this block ends; `debugLevel`
    // above zero fills the absorbed header with 0xDEADDEAD. No caller
    // remains in this build; MemHeap::MergeFreeBlocks does the same work.
    bool AttemptMerge(FreeBlock* next, int debugLevel);  // 0x37EC30

    // Field names are not in the reference map.
    unsigned int mSizeWords;
    // The free list's stamp when the block was freed; LRU fit prefers the
    // smallest.
    int mTime;
    FreeBlock* mNext;
    FreeBlock* mPrev;
};

static_assert(sizeof(FreeBlock) == 24);

// The 64-bit header in front of an allocation. In an internal heap the
// caller's memory starts after it; an external heap keeps it in its
// bookkeeping memory and maps the allocation into the external range.
struct AllocBlock {
    // Field names are not in the reference map.
    // The words between the start of the claimed block and this header.
    unsigned long mPadWords : 16;
    // Kept by Alloc and never set in this build.
    unsigned long mReserved16 : 16;
    // The claimed block's size, padding included.
    unsigned long mSizeWords : 28;
    // The strategy the block was allocated with; Free searches the free
    // list from the tail for kLastFit blocks.
    unsigned long mStrategy : 2;
    // Kept by Alloc and never set in this build.
    unsigned long mReserved62 : 2;
};

static_assert(sizeof(AllocBlock) == 8);

// What MemFreeBlockStats reports for a heap: the free list's fragmentation
// around its biggest block, the heap's size and its free bytes. Field names
// are not in the reference map.
struct MemHeapStats {
    // The biggest free block's index in the free list, and the number of
    // free blocks after it.
    int mLeftFrags;
    int mRightFrags;
    // Always zero in this build.
    unsigned long mLeftAllocs;
    unsigned long mRightAllocs;
    // Filled by the caller, not by FreeBlockStats.
    unsigned long mSize;
    unsigned long mFreeBytes;
    unsigned long mBiggestFree;
};

static_assert(sizeof(MemHeapStats) == 0x30);

// A free-list heap over a fixed block of memory (os/MemHeap.o). Sizes are
// counted in 4-byte words. An external heap (mExternalStart set) hands out
// memory in another range, such as GPU memory, at a granularity of
// 1 << mShift words per bookkeeping word, keeping its headers apart.
class MemHeap {
public:
    // How Alloc picks a free block.
    enum Strategy {
        kFirstFit = 0,
        kBestFit = 1,
        kLRUFit = 2,
        // From the end of the free list, placing the allocation at the end
        // of the block; temporary allocations use it.
        kLastFit = 3,
    };

    // The chosen free block and how far into it the aligned allocation
    // starts. Field names are not in the reference map.
    struct FreeBlockInfo {
        FreeBlock* mBlock;
        FreeBlock* mPrev;
        unsigned long mSizeWords;
        unsigned long mPadWords;
    };

    // The size Alloc is asked for, in words: the bytes rounded up to the
    // granularity, the internal header added and at least a free block's
    // size.
    unsigned long GetSizeWords(unsigned long bytes);  // 0x37C750
    // The alignment exponent in words for a byte alignment.
    int GetAlignWords(int align);  // 0x37C790
    // Links the block between `prev` and `next` with its size and stamp.
    void InsertFreeBlock(FreeBlock* block, unsigned long sizeWords, FreeBlock* prev,
                         FreeBlock* next, int time);  // 0x37C7D0
    // Fills the fragmentation and free-byte fields of `stats`.
    void FreeBlockStats(MemHeapStats& stats);  // 0x37C800
    // Takes `sizeWords` words at `mem` as one free block. The map's
    // signature has a bool after the size, which this build lacks.
    void Init(const char* name, unsigned long heapIndex, int* mem, unsigned long sizeWords,
              Strategy strategy, int debugLevel, bool allowTemp, int* external,
              unsigned long externalSizeWords, int minSplitWords, bool tracked);  // 0x37C8A0
    // Reserves room for 4096 batched frees.
    void InitBatchedFreeList();  // 0x37CA10
    void FirstFit(unsigned long sizeWords, int align, FreeBlockInfo& info);  // 0x37CAC0
    // The words from `block` to the next `align` boundary after `extra`
    // header words. Name not in the reference map.
    unsigned long AlignPadding(int* block, int align, unsigned long extra);  // 0x37CBB0
    void BestFit(unsigned long sizeWords, int align, FreeBlockInfo& info);  // 0x37CC00
    void LRUFit(unsigned long sizeWords, int align, FreeBlockInfo& info);   // 0x37CCB0
    void LastFit(unsigned long sizeWords, int align, FreeBlockInfo& info);  // 0x37CD70
    // The padding that places `sizeWords` words, aligned, at the end of the
    // free block. Name not in the reference map.
    long LastFitPadding(FreeBlock* block, unsigned long sizeWords, int align,
                        unsigned long extra);  // 0x37CE70
    // Allocates `sizeWords` words aligned to 1 << `align` words, from the
    // fallback heap when this one has no room. `allocated` receives the
    // words taken. A failure is reported, and `failed` set, only when
    // `report` is set. The map has Alloc(unsigned long, int, unsigned long,
    // unsigned long&, bool).
    int* Alloc(unsigned long sizeWords, int align, unsigned long& allocated, bool report,
               bool& failed);  // 0x37CEE0
    // The external address of an internal one and back.
    int* ExternalAddr(int* addr);  // 0x37DB30
    int* InternalAddr(int* addr);  // 0x37DB50
    // The bytes from `addr` to the end of its allocation, or zero when the
    // heap does not own it.
    unsigned long AllocSize(int* addr);  // 0x37DB80
    bool OwnsAddr(int* addr);  // 0x37DC40
    void FindFreeNeighbors(AllocBlock* block, FreeBlock*& prev, FreeBlock*& next);  // 0x37DC90
    // Frees the allocation at `addr` when the heap owns it, or queues it
    // when `batch` is set; `sizeWords` receives its size. The map has
    // Free(int*, unsigned long&, unsigned long&, bool).
    bool Free(int* addr, unsigned long& sizeWords, bool batch);  // 0x37DD00
    // Returns the queued allocations to the free list in one pass and
    // merges adjacent free blocks.
    void ProcessBatchedFreeList();  // 0x37E120
    // As FreeBlock::AttemptMerge, keeping the free list's tail. Name not in
    // the reference map.
    bool MergeFreeBlocks(FreeBlock* block, FreeBlock* next);  // 0x37E500
    // Walks the blocks; the checks are compiled out in this build.
    void CheckConsistency();  // 0x37E570
    // Writes the heap and its blocks as JSON; `freeOnly` leaves out the
    // allocations.
    void Print(TextStream& stream, bool freeOnly);  // 0x37E5D0
    void SetFallback(MemHeap* fallback);  // 0x37EC90

    // Field names are not in the reference map.
    FreeBlock* mFreeHead;
    FreeBlock* mFreeTail;
    int* mStart;
    MemHeap* mFallback;
    const char* mName;
    unsigned long mSizeWords;
    // Zeroed by Init and counted in the heap's reported size; not otherwise
    // used in this build. Weak.
    unsigned long mExtraBytes;
    unsigned long mHeapIndex;
    // Above zero, free memory is filled with 0xDEADDEAD and new
    // allocations with 0xABCDABCD.
    int mDebugLevel;
    Strategy mStrategy;
    // Whether temporary allocations may use the heap; MemAlloc moves them
    // down the heap stack to one that allows them. Weak.
    bool mAllowTemp;
    bool mTracked;
    int* mExternalStart;
    unsigned long mExternalSizeWords;
    int mShift;
    // Leftovers of at most this many words stay with the allocation.
    int mMinSplitWords;
    // The memory ranges the heap owns. Init sets the first.
    struct Region {
        int* mStart;
        unsigned long mSizeWords;
    } mRegions[32];
    unsigned long mNumRegions;
    eastl::vector<AllocBlock*> mBatchedFrees;
};

static_assert(offsetof(MemHeap, mName) == 0x20);
static_assert(offsetof(MemHeap, mDebugLevel) == 0x40);
static_assert(offsetof(MemHeap, mStrategy) == 0x44);
static_assert(offsetof(MemHeap, mAllowTemp) == 0x48);
static_assert(offsetof(MemHeap, mExternalStart) == 0x50);
static_assert(offsetof(MemHeap, mShift) == 0x60);
static_assert(offsetof(MemHeap, mRegions) == 0x68);
static_assert(offsetof(MemHeap, mNumRegions) == 0x268);
static_assert(offsetof(MemHeap, mBatchedFrees) == 0x270);
static_assert(sizeof(MemHeap) == 0x290);

// Orders batched frees from the highest address down. The binary passes it
// to the sort as bool (*)(AllocBlock const*, AllocBlock const*).
bool BatchedFreesCompare(const AllocBlock* a, const AllocBlock* b);  // 0x37EC20

// Stamps free blocks in the order they are freed. At 0x1A01BB8. Name not in
// the reference map.
extern int gFreeBlockTime;
