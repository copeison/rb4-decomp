#include "os/memory/MemHeap.h"

#include <_pthread.h>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "os/memory/MemTrack.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Sort.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/threading/Thread.h"

int gFreeBlockTime;

namespace {

// Fill patterns for freed and newly allocated words in debug heaps. Names
// not in the reference map.
constexpr unsigned int kFreeFill = 0xDEADDEAD;
constexpr unsigned int kAllocFill = 0xABCDABCD;

// The smallest power of two at least `value`, as an exponent; the binary
// rounds the float's exponent up. Name not in the reference map.
int CeilLog2(int value) {
    const float f = static_cast<float>(value);
    unsigned int bits;
    memcpy(&bits, &f, sizeof(bits));
    return static_cast<int>(((bits + 0x7FFFFF) >> 23) - 127);
}

// Fills the words in [begin, end).
void FillWords(unsigned int* begin, unsigned int* end, unsigned int pattern) {
    for (; begin < end; ++begin) {
        *begin = pattern;
    }
}

// Writes one run of equal-sized allocations of Print's block list. Name
// not in the reference map.
void PrintAllocRun(TextStream& stream, int* addr, unsigned long size, int count,
                   AllocInfo* info) {  // 0x37EB20
    if (count <= 0) {
        return;
    }
    stream << "      {\n";
    stream << "         \"alloc\": true,\n";
    stream << "         \"address\": " << reinterpret_cast<unsigned long>(addr) << ",\n";
    stream << "         \"size\": " << size << ",\n";
    if (count != 1) {
        stream << "         \"num\": " << count << ",\n";
    }
    if (info != nullptr) {
        stream << *info;
    }
    stream << "      }";
}

}  // namespace

// Reconstructed from eboot.elf at 0x37C750.
unsigned long MemHeap::GetSizeWords(unsigned long bytes) {
    const int shift = mShift;
    const unsigned long units = (bytes + (1UL << shift) - 1) >> shift;
    unsigned long words = ((units + 3) >> 2) + (mExternalStart == nullptr ? 2 : 0);
    if (words < 6) {
        words = 6;
    }
    return words << shift;
}

// Reconstructed from eboot.elf at 0x37C790.
int MemHeap::GetAlignWords(int align) {
    int minimum;
    if (mExternalStart != nullptr) {
        minimum = mShift;
    } else {
        minimum = 0;
        if (align == 0) {
            return 2;
        }
    }
    const int words = CeilLog2(align) - 2;
    return minimum >= words ? minimum : words;
}

// Reconstructed from eboot.elf at 0x37C7D0.
void MemHeap::InsertFreeBlock(FreeBlock* block, unsigned long sizeWords, FreeBlock* prev,
                              FreeBlock* next, int time) {
    block->mSizeWords = sizeWords;
    block->mTime = time;
    block->mNext = next;
    block->mPrev = prev;
    (prev != nullptr ? prev->mNext : mFreeHead) = block;
    (next != nullptr ? next->mPrev : mFreeTail) = block;
}

// Reconstructed from eboot.elf at 0x37C800.
void MemHeap::FreeBlockStats(MemHeapStats& stats) {
    int index = -1;
    unsigned long freeBytes = 0;
    unsigned long biggest = 0;
    int biggestIndex = -1;
    for (FreeBlock* block = mFreeHead; block != nullptr; block = block->mNext) {
        ++index;
        const unsigned long bytes = 4UL * block->mSizeWords;
        freeBytes += bytes;
        if (biggest < bytes) {
            biggest = bytes;
            biggestIndex = index;
        }
    }
    stats.mBiggestFree = biggest << mShift;
    stats.mFreeBytes = freeBytes << mShift;
    stats.mLeftFrags = biggestIndex;
    stats.mRightFrags = index - biggestIndex;
    stats.mLeftAllocs = 0;
    stats.mRightAllocs = 0;
}

// Reconstructed from eboot.elf at 0x37C8A0.
void MemHeap::Init(const char* name, unsigned long heapIndex, int* mem, unsigned long sizeWords,
                   Strategy strategy, int debugLevel, bool allowTemp, int* external,
                   unsigned long externalSizeWords, int minSplitWords, bool tracked) {
    mName = name;
    mHeapIndex = heapIndex;
    mSizeWords = sizeWords;
    mExtraBytes = 0;
    mStart = mem;
    mStrategy = strategy;
    mDebugLevel = debugLevel;
    mAllowTemp = allowTemp;
    mExternalSizeWords = externalSizeWords;
    mExternalStart = external;
    mMinSplitWords = minSplitWords;
    mTracked = tracked;
    if (externalSizeWords != 0) {
        mShift = CeilLog2(static_cast<int>(externalSizeWords / sizeWords));
    } else {
        // An internal heap starts on a 16-byte boundary.
        int* aligned = reinterpret_cast<int*>((reinterpret_cast<unsigned long>(mem) + 15) & ~15UL);
        sizeWords -= aligned - mem;
        mem = aligned;
        mSizeWords = sizeWords;
        mStart = mem;
        mShift = 0;
    }
    auto* block = reinterpret_cast<FreeBlock*>(mem);
    block->mSizeWords = sizeWords;
    block->mTime = gFreeBlockTime++;
    block->mNext = nullptr;
    block->mPrev = nullptr;
    mFreeHead = block;
    mFreeTail = block;
    if (debugLevel > 0) {
        FillWords(reinterpret_cast<unsigned int*>(mem) + 6,
                  reinterpret_cast<unsigned int*>(mem) + static_cast<unsigned int>(sizeWords),
                  kFreeFill);
    }
    mRegions[0].mStart = mem;
    mRegions[0].mSizeWords = sizeWords;
    mNumRegions = 1;
}

// Reconstructed from eboot.elf at 0x37CA10.
void MemHeap::InitBatchedFreeList() {
    mBatchedFrees.reserve(0x1000);
}

// Reconstructed from eboot.elf at 0x37CAC0.
void MemHeap::FirstFit(unsigned long sizeWords, int align, FreeBlockInfo& info) {
    const unsigned long extra = mExternalStart == nullptr ? 2 : 0;
    for (FreeBlock* block = mFreeHead; block != nullptr; block = block->mNext) {
        const unsigned long pad = AlignPadding(reinterpret_cast<int*>(block), align, extra);
        if (block->mSizeWords >= pad + sizeWords) {
            info.mSizeWords = block->mSizeWords;
            info.mPadWords = pad;
            info.mBlock = block;
            info.mPrev = block->mPrev;
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x37CBB0.
unsigned long MemHeap::AlignPadding(int* block, int align, unsigned long extra) {
    const unsigned long units = (reinterpret_cast<unsigned long>(ExternalAddr(block)) >> (mShift + 2)) + extra;
    return ((units + (1UL << align) - 1) >> align << align) - units;
}

// Reconstructed from eboot.elf at 0x37CC00.
void MemHeap::BestFit(unsigned long sizeWords, int align, FreeBlockInfo& info) {
    const unsigned long extra = mExternalStart == nullptr ? 2 : 0;
    for (FreeBlock* block = mFreeHead; block != nullptr; block = block->mNext) {
        const unsigned long size = block->mSizeWords;
        const unsigned long pad = AlignPadding(reinterpret_cast<int*>(block), align, extra);
        if (size >= pad + sizeWords && size < info.mSizeWords) {
            info.mSizeWords = size;
            info.mPadWords = pad;
            info.mBlock = block;
            info.mPrev = block->mPrev;
        }
    }
}

// Reconstructed from eboot.elf at 0x37CCB0.
void MemHeap::LRUFit(unsigned long sizeWords, int align, FreeBlockInfo& info) {
    const unsigned long extra = mExternalStart == nullptr ? 2 : 0;
    int oldest = 0x7FFFFFFF;
    for (FreeBlock* block = mFreeHead; block != nullptr; block = block->mNext) {
        const unsigned long size = block->mSizeWords;
        if (block->mTime < oldest) {
            const unsigned long pad = AlignPadding(reinterpret_cast<int*>(block), align, extra);
            if (size >= pad + sizeWords) {
                info.mSizeWords = size;
                info.mPadWords = pad;
                info.mBlock = block;
                oldest = block->mTime;
                info.mPrev = block->mPrev;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x37CD70.
void MemHeap::LastFit(unsigned long sizeWords, int align, FreeBlockInfo& info) {
    const unsigned long extra = mExternalStart == nullptr ? 2 : 0;
    for (FreeBlock* block = mFreeTail; block != nullptr; block = block->mPrev) {
        const long pad = LastFitPadding(block, sizeWords, align, extra);
        if (pad >= 0) {
            info.mSizeWords = block->mSizeWords;
            info.mPadWords = pad;
            info.mBlock = block;
            info.mPrev = block->mPrev;
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x37CE70.
long MemHeap::LastFitPadding(FreeBlock* block, unsigned long sizeWords, int align,
                             unsigned long extra) {
    const int shift = mShift + 2;
    int* end = ExternalAddr(reinterpret_cast<int*>(block) + block->mSizeWords);
    const unsigned long start = reinterpret_cast<unsigned long>(ExternalAddr(reinterpret_cast<int*>(block))) >> shift;
    return static_cast<long>((((reinterpret_cast<unsigned long>(end) >> shift) - sizeWords) >> align << align) - extra - start);
}

// Reconstructed from eboot.elf at 0x37CEE0.
int* MemHeap::Alloc(unsigned long sizeWords, int align, unsigned long& allocated, bool report,
                    bool& failed) {
    failed = false;
    const int shift = mShift;
    align -= shift;
    sizeWords = (sizeWords + (1UL << shift) - 1) >> shift;

    FreeBlockInfo info;
    info.mBlock = nullptr;
    info.mPrev = nullptr;
    info.mSizeWords = 0x7FFFFFFF;
    info.mPadWords = 0x7FFFFFFF;
    switch (mStrategy) {
    case kFirstFit:
        FirstFit(sizeWords, align, info);
        break;
    case kBestFit:
        BestFit(sizeWords, align, info);
        break;
    case kLRUFit:
        LRUFit(sizeWords, align, info);
        break;
    case kLastFit:
        LastFit(sizeWords, align, info);
        break;
    }

    if (info.mBlock == nullptr) {
        if (mFallback != nullptr) {
            bool fallbackFailed = false;
            return mFallback->Alloc(sizeWords, align, allocated, report, fallbackFailed);
        }
        // The release build reports the failure and carries on.
        MemHeapStats stats;
        FreeBlockStats(stats);
        if (!report) {
            return nullptr;
        }
        SetMemoryFailure();
        if (scePthreadSelf() != Thread::s_MainThreadID && Thread::s_MainThreadID != nullptr) {
            gInsideMemFunc = false;
            CritSec* lock = gMemLock;
            int held;
            do {
                held = lock->mEntryCount--;
                scePthreadMutexUnlock(&lock->mCritSec);
            } while (held > 1);
        }
        StackString<2048> msg;
        char want[32] = {};
        char size[32] = {};
        char leftFrags[32] = {};
        char rightFrags[32] = {};
        char biggest[32] = {};
        char freeBytes[32] = {};
        FormatString format(
            "Allocation failure, heap \"%s\", want %s bytes\n"
            "     size:       %10s\n"
            "   lFrags:       %10s\n"
            "   rFrags:       %10s\n"
            "   Biggest Block:%10s\n"
            "   Free Bytes:   %10s\n");
        format << mName << PrintIntWithCommas(sizeWords << (mShift + 2), want, sizeof(want))
               << PrintIntWithCommas(stats.mSize, size, sizeof(size))
               << PrintIntWithCommas(stats.mLeftFrags, leftFrags, sizeof(leftFrags))
               << PrintIntWithCommas(stats.mRightFrags, rightFrags, sizeof(rightFrags))
               << PrintIntWithCommas(stats.mBiggestFree, biggest, sizeof(biggest))
               << PrintIntWithCommas(stats.mFreeBytes, freeBytes, sizeof(freeBytes));
        msg << format.Str();
        MemPrintOverview(-3, msg);
    }

    FreeBlock* block = info.mBlock;
    unsigned long blockWords = info.mSizeWords;
    unsigned long pad = info.mPadWords;
    if (pad >= 0x81) {
        // A large padding stays free as a block of its own.
        auto* rest = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(block) + pad);
        blockWords -= pad;
        InsertFreeBlock(rest, blockWords, block, block->mNext, block->mTime);
        InsertFreeBlock(block, pad, info.mPrev, rest, block->mTime);
        block = rest;
        pad = 0;
    }

    unsigned long used = pad + sizeWords;
    if (blockWords - used <= static_cast<unsigned long>(static_cast<long>(mMinSplitWords))) {
        // Too little is left to split off: take the whole block.
        (block->mPrev != nullptr ? block->mPrev->mNext : mFreeHead) = block->mNext;
        (block->mNext != nullptr ? block->mNext->mPrev : mFreeTail) = block->mPrev;
        used = blockWords;
    } else {
        InsertFreeBlock(reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(block) + used),
                        blockWords - used, block->mPrev, block->mNext, block->mTime);
    }

    auto* header = reinterpret_cast<AllocBlock*>(reinterpret_cast<int*>(block) + pad);
    header->mPadWords = pad;
    header->mSizeWords = used;
    header->mStrategy = mStrategy & 3;
    int* result = reinterpret_cast<int*>(header + (mExternalStart == nullptr ? 1 : 0));
    if (mDebugLevel > 0) {
        if (static_cast<unsigned short>(pad) != 0) {
            memset(block, 0, pad * 4);
        }
        int* end = reinterpret_cast<int*>(header) - header->mPadWords + header->mSizeWords;
        FillWords(reinterpret_cast<unsigned int*>(ExternalAddr(result)),
                  reinterpret_cast<unsigned int*>(ExternalAddr(end)), kAllocFill);
    }
    allocated = header->mSizeWords << mShift;
    return ExternalAddr(result);
}

// Reconstructed from eboot.elf at 0x37DB30.
int* MemHeap::ExternalAddr(int* addr) {
    if (mExternalStart != nullptr) {
        return mExternalStart + ((addr - mStart) << mShift);
    }
    return addr;
}

// Reconstructed from eboot.elf at 0x37DB50.
int* MemHeap::InternalAddr(int* addr) {
    if (mExternalStart != nullptr) {
        return mStart + ((addr - mExternalStart) >> mShift);
    }
    return addr;
}

// Reconstructed from eboot.elf at 0x37DB80.
unsigned long MemHeap::AllocSize(int* addr) {
    if (!OwnsAddr(addr)) {
        return 0;
    }
    int* internal = InternalAddr(addr);
    auto* header = reinterpret_cast<AllocBlock*>(internal + (mExternalStart != nullptr ? 0 : -2));
    int* end = reinterpret_cast<int*>(header) - header->mPadWords + header->mSizeWords;
    return reinterpret_cast<char*>(ExternalAddr(end)) - reinterpret_cast<char*>(addr);
}

// Reconstructed from eboot.elf at 0x37DC40.
bool MemHeap::OwnsAddr(int* addr) {
    for (unsigned long i = 0; i < mNumRegions; ++i) {
        if (mRegions[i].mStart <= addr && mRegions[i].mStart + mRegions[i].mSizeWords > addr) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x37DC90.
void MemHeap::FindFreeNeighbors(AllocBlock* block, FreeBlock*& prev, FreeBlock*& next) {
    FreeBlock* before;
    FreeBlock* after;
    if (block->mStrategy == kLastFit) {
        after = nullptr;
        before = mFreeTail;
        while (before != nullptr && reinterpret_cast<FreeBlock*>(block) < before) {
            after = before;
            before = before->mPrev;
        }
    } else {
        before = nullptr;
        after = mFreeHead;
        while (after != nullptr && after < reinterpret_cast<FreeBlock*>(block)) {
            before = after;
            after = after->mNext;
        }
    }
    next = after;
    prev = before;
}

// Reconstructed from eboot.elf at 0x37DD00.
bool MemHeap::Free(int* addr, unsigned long& sizeWords, bool batch) {
    bool external;
    if (mExternalStart != nullptr) {
        if (addr < mExternalStart || addr >= mExternalStart + mExternalSizeWords) {
            return false;
        }
        external = true;
        addr = InternalAddr(addr);
    } else {
        if (!OwnsAddr(addr)) {
            return false;
        }
        external = false;
    }
    auto* header = reinterpret_cast<AllocBlock*>(addr + (external ? 0 : -2));
    sizeWords = header->mSizeWords;
    if (batch) {
        if (mBatchedFrees.size() == 0x1000) {
            ProcessBatchedFreeList();
        }
        mBatchedFrees.push_back(header);
        return true;
    }

    FreeBlock* prev;
    FreeBlock* next;
    FindFreeNeighbors(header, prev, next);
    const unsigned int words = header->mSizeWords;
    auto* block = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(header) - header->mPadWords);
    InsertFreeBlock(block, words, prev, next, gFreeBlockTime++);
    if (mDebugLevel > 0 && words >= 7) {
        FillWords(reinterpret_cast<unsigned int*>(block) + 6,
                  reinterpret_cast<unsigned int*>(block) + words, kFreeFill);
    }
    if (next != nullptr) {
        MergeFreeBlocks(block, next);
    }
    if (prev != nullptr) {
        MergeFreeBlocks(prev, block);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x37E120.
void MemHeap::ProcessBatchedFreeList() {
    eastl::sort(mBatchedFrees.begin(), mBatchedFrees.end(), BatchedFreesCompare);

    // The batch is sorted from the highest address down, so its back is the
    // lowest. A block below the first free block becomes the new head.
    if (!mBatchedFrees.empty()) {
        AllocBlock* header = mBatchedFrees.back();
        if (reinterpret_cast<FreeBlock*>(header) < mFreeHead) {
            mBatchedFrees.pop_back();
            auto* block = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(header) - header->mPadWords);
            InsertFreeBlock(block, header->mSizeWords, nullptr, mFreeHead, gFreeBlockTime++);
        }
    }
    // Each block that falls between a free block and the next is linked
    // after it.
    FreeBlock* last = nullptr;
    for (FreeBlock* free = mFreeHead; free != nullptr; free = free->mNext) {
        last = free;
        if (mBatchedFrees.empty()) {
            break;
        }
        AllocBlock* header = mBatchedFrees.back();
        auto* pending = reinterpret_cast<FreeBlock*>(header);
        if (pending > free && (free->mNext == nullptr || pending < free->mNext)) {
            mBatchedFrees.pop_back();
            auto* block = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(header) - header->mPadWords);
            InsertFreeBlock(block, header->mSizeWords, free, free->mNext, gFreeBlockTime++);
        }
    }
    // The rest lie past the last free block.
    while (!mBatchedFrees.empty()) {
        AllocBlock* header = mBatchedFrees.back();
        mBatchedFrees.pop_back();
        auto* block = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(header) - header->mPadWords);
        InsertFreeBlock(block, header->mSizeWords, last, last->mNext, gFreeBlockTime++);
        last = block;
    }
    // Merge the neighbours the batch created.
    FreeBlock* block = mFreeHead;
    while (block != nullptr && block->mNext != nullptr) {
        if (!MergeFreeBlocks(block, block->mNext)) {
            block = block->mNext;
        }
    }
}

// Reconstructed from eboot.elf at 0x37E500.
bool MemHeap::MergeFreeBlocks(FreeBlock* block, FreeBlock* next) {
    auto* end = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(block) + block->mSizeWords);
    if (end != next) {
        return false;
    }
    block->mSizeWords += next->mSizeWords;
    if (block->mTime < next->mTime) {
        block->mTime = next->mTime;
    }
    block->mNext = next->mNext;
    if (next->mNext != nullptr) {
        next->mNext->mPrev = block;
    }
    if (mDebugLevel > 0) {
        FillWords(reinterpret_cast<unsigned int*>(next), reinterpret_cast<unsigned int*>(next + 1),
                  kFreeFill);
    }
    if (mFreeTail == next) {
        mFreeTail = block;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x37E570.
void MemHeap::CheckConsistency() {
    if (mDebugLevel == 0 || static_cast<long>(mSizeWords) <= 0) {
        return;
    }
    unsigned int* word = reinterpret_cast<unsigned int*>(mStart);
    unsigned int* end = word + mSizeWords;
    FreeBlock* free = mFreeHead;
    do {
        unsigned int size;
        if (free != nullptr && reinterpret_cast<FreeBlock*>(word) == free) {
            size = free->mSizeWords;
            free = free->mNext;
        } else {
            // Skip the padding to the allocation's header.
            unsigned int* header = word;
            while (*header++ == 0) {
            }
            size = *header & 0xFFFFFFF;
        }
        word += size;
    } while (word < end);
}

// Reconstructed from eboot.elf at 0x37E5D0.
void MemHeap::Print(TextStream& stream, bool freeOnly) {
    const bool internal = mExternalStart == nullptr;
    stream << "{\n";
    stream << "   \"heap_name\": \"" << mName << "\",\n";
    stream << "   \"heap_index\": " << mHeapIndex << ",\n";
    stream << "   \"start_addr\": " << reinterpret_cast<unsigned long>(mStart) << ",\n";
    // The binary prints the word count followed by a 2, not the byte size.
    stream << "   \"size\": " << mSizeWords << 2 << ",\n";
    MemHeapStats stats;
    FreeBlockStats(stats);
    // mSize is not filled by FreeBlockStats; the binary prints whatever the
    // stack held.
    stream << "   \"size\": " << stats.mSize << ",\n";
    stream << "   \"lfrags\": " << stats.mLeftFrags << ",\n";
    stream << "   \"rfrags\": " << stats.mRightFrags << ",\n";
    stream << "   \"lallocs\": " << stats.mLeftAllocs << ",\n";
    stream << "   \"rallocs\": " << stats.mRightAllocs << ",\n";
    stream << "   \"bytes_free\": " << stats.mFreeBytes << ",\n";
    stream << "   \"blocks\": [\n";
    if (static_cast<long>(mSizeWords) > 0) {
        int* block = mStart;
        int* end = mStart + mSizeWords;
        FreeBlock* free = mFreeHead;
        bool first = true;
        // A run of allocations of one size, printed as one entry.
        int* runAddr = nullptr;
        unsigned long runSize = 0;
        int runCount = 0;
        AllocInfo* runInfo = nullptr;
        do {
            unsigned int words;
            if (free != nullptr && block == reinterpret_cast<int*>(free)) {
                if (runCount != 0) {
                    if (!first) {
                        stream << ",\n";
                    }
                    PrintAllocRun(stream, runAddr, runSize, runCount, runInfo);
                    stream << ",\n";
                } else if (!first) {
                    stream << ",\n";
                }
                words = free->mSizeWords;
                const unsigned long bytes = 4UL * words;
                stream << "      {\n";
                stream << "         \"free\": true,\n";
                stream << "         \"address\": " << reinterpret_cast<unsigned long>(block) << ",\n";
                stream << "         \"size\": " << bytes << ",\n";
                stream << "         \"time\": " << static_cast<unsigned int>(free->mTime);
                if (bytes >= 100000) {
                    stream << ",\n         \"big_free\": true";
                }
                stream << "\n      }";
                free = free->mNext;
                runSize = 0;
                runCount = 0;
                first = false;
            } else {
                // Skip the padding to the allocation's header.
                int* header = block - 1;
                do {
                    ++header;
                } while (*header == 0);
                words = header[1] & 0xFFFFFFF;
                if (!freeOnly) {
                    int* addr = ExternalAddr(header + (internal ? 2 : 0));
                    AllocInfo* info = MemTrackGetInfo(addr);
                    const unsigned long bytes = 4UL * words;
                    if (bytes == runSize) {
                        ++runCount;
                    } else {
                        if (runCount != 0) {
                            if (!first) {
                                stream << ",\n";
                            }
                            PrintAllocRun(stream, runAddr, runSize, runCount, runInfo);
                            first = false;
                        }
                        runSize = bytes;
                        runCount = 1;
                        runInfo = info;
                        runAddr = addr;
                    }
                }
            }
            block += words;
        } while (block < end);
        if (runCount != 0) {
            if (!first) {
                stream << ",\n";
            }
            PrintAllocRun(stream, runAddr, runSize, runCount, runInfo);
        }
    }
    stream << "\n   ]\n";
    stream << "}";
}

// Reconstructed from eboot.elf at 0x37EC20.
bool BatchedFreesCompare(const AllocBlock* a, const AllocBlock* b) {
    return b < a;
}

// Reconstructed from eboot.elf at 0x37EC30.
bool FreeBlock::AttemptMerge(FreeBlock* next, int debugLevel) {
    auto* end = reinterpret_cast<FreeBlock*>(reinterpret_cast<int*>(this) + mSizeWords);
    if (end != next) {
        return false;
    }
    mSizeWords += next->mSizeWords;
    if (mTime < next->mTime) {
        mTime = next->mTime;
    }
    mNext = next->mNext;
    if (next->mNext != nullptr) {
        next->mNext->mPrev = this;
    }
    if (debugLevel > 0) {
        FillWords(reinterpret_cast<unsigned int*>(next), reinterpret_cast<unsigned int*>(next + 1),
                  kFreeFill);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x37EC90.
void MemHeap::SetFallback(MemHeap* fallback) {
    mFallback = fallback;
}
