#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "os/profiling/PerfTimer.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Std.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class RndContext;
class TextStream;

// GPU timing statistics, embedded in RndDevice. Each BeginStatBlock and
// EndStatBlock pair records a GPU query against a named statistic; EndFrame
// resolves the queries of the oldest of four frame slots, smooths the
// timings, and sums the statistics into one total per name.
//
// The map's build keeps the statistics in an eastl::map<Symbol, StatBlock>
// and gives the class a vtable. This build has no vtable: the manager holds
// two vectors sorted by Symbol address, one of every statistic (Stat) and one
// of the per-name totals (StatBlock).
class RndGpuStatsMgr {
public:
    // Storage of an eastl::vector<T> whose allocator word the constructor
    // zeroes. Name not in the reference map.
    template <typename T>
    struct Array {
        ~Array() {
            if (mBegin != nullptr) {
                Free();
            }
        }

        // Inserts before `position`, doubling the storage when it is full.
        void Insert(T* position, T value);
        void PushBack(T value) {
            Insert(mEnd, value);
        }
        void Free() {
            HmxAllocator::gStlAllocator.deallocate(
                mBegin,
                static_cast<unsigned long>(
                    reinterpret_cast<char*>(mCapacity) -
                    reinterpret_cast<char*>(mBegin)));
        }

        T* mBegin;
        T* mEnd;
        T* mCapacity;
        void* mAllocator;
    };

    // Timings of one resolved frame. Name not in the reference map; the field
    // names are not either.
    struct Frame {
        void Reset();

        int mQueryCount;
        float mLastSeconds;
        float mSeconds;
        float mAverageQueryCount;
        float mAverageSeconds;
        float mWorstSeconds;
        int mSampleCount;
        unsigned int mUnknown28;
        unsigned long mCounters[6];
    };

    // One timed scope. Its name is the scope name; its full name prefixes
    // the names of the enclosing scopes. The vtable is at 0x192E120. Name
    // not in the reference map; the field names are not either.
    class Stat : public PerfTimerBase {
    public:
        // Reads the statistic's settings from the "gpu_timer" system
        // configuration.
        Stat(Symbol name, Stat* parent);  // 0x6F2930
        // Slots 0 and 1 at 0x62D270 and 0x62D320. The manager's destructor
        // releases each statistic through the deleting destructor; the
        // complete destructor is inlined into it for the StatBlock totals.
        ~Stat() override;

        int _GetCount(unsigned long frame) const override;           // 0x62D3D0
        float _GetAverageCount(unsigned long frame) const override;  // 0x62D3E0
        float _GetMs(unsigned long frame) const override;            // 0x62D400
        float _GetAverageMs(unsigned long frame) const override;     // 0x62D420
        float _GetWorstMs(unsigned long frame) const override;       // 0x62D440

        unsigned int mUnknown52;
        Array<unsigned long> mQueryKeys[4];  // One per frame slot.
        Frame mFrames[2];
        Symbol mFullNameSym;
    };

    // Total of every statistic sharing one name, and the statistics
    // themselves. The map's StatBlock plays the same role.
    struct StatBlock {
        DELETE_OVERLOAD

        StatBlock(Symbol name, Stat* parent);

        Array<Stat*> mChildren;  // Name not in the reference map.
        Stat mTotal;             // Name not in the reference map.
    };

    RndGpuStatsMgr();   // 0x62AAE0
    ~RndGpuStatsMgr();  // 0x62ABA0

    // Creates the "GPU Total" block and one block per budget category.
    // Name not in the reference map.
    void Init();  // 0x62ACB0
    // Returns the query key, or -1 when no GPU statistics backend is
    // installed. The map does not give the return type.
    long BeginStatBlock(RndContext& context, const char* name);  // 0x62AF80
    // The map's signature is EndStatBlock(RndContext&, unsigned long); the
    // binary ignores negative keys.
    void EndStatBlock(RndContext& context, long key);  // 0x62B5B0
    void EndFrame();                                    // 0x62B960

    // Not located in this build.
    float GetMs(Symbol name);
    float GetAverageMs(Symbol name, unsigned long frames);
    float GetWorstMs(Symbol name, unsigned long frames);
    void ResetTimers();
    void PrintCSV(TextStream& stream);

    void _GatherStats();  // 0x62B9E0
    // Inlined into EndFrame.
    void _NextFrame();

    // Not located in this build.
    void _Print(TextStream& stream);
    void _PrintHeader(TextStream& stream, const char* title);
    void _PrintSeparator(TextStream& stream, int width);
    void _PrintStat(TextStream& stream, unsigned long value);
    void _PrintTimer(TextStream& stream, float ms);

    // Returns the statistic for `fullName`, creating it and adding it to the
    // block for `name`. Name not in the reference map.
    Stat* _FindOrCreateStat(
        const char* name,
        const char* fullName,
        Stat* parent);  // 0x62B2C0
    // Name not in the reference map.
    Stat* _FindStat(Symbol fullName);

    // Field names are not in the reference map.
    Array<Stat*> mStats;            // Sorted by full-name Symbol address.
    StatBlock* mTotalBlock;         // "GPU Total".
    Array<StatBlock*> mStatBlocks;  // Sorted by name Symbol address.
    unsigned long mNextKey;
    unsigned long mFrameSlot;       // Frame slot receiving new queries.
    unsigned char mUnknown88[8];
    unsigned long mResolvedFrame;   // Index into Stat::mFrames.
    void* mBackend;                 // Statistics are recorded when set.
    CritSec mCritSec;
};

static_assert(sizeof(RndGpuStatsMgr::Array<void*>) == 32);
static_assert(sizeof(RndGpuStatsMgr::Frame) == 80);
static_assert(offsetof(RndGpuStatsMgr::Stat, mUnknown52) == 52);
static_assert(offsetof(RndGpuStatsMgr::Stat, mQueryKeys) == 56);
static_assert(offsetof(RndGpuStatsMgr::Stat, mFrames) == 184);
static_assert(offsetof(RndGpuStatsMgr::Stat, mFullNameSym) == 344);
static_assert(sizeof(RndGpuStatsMgr::Stat) == 352);
static_assert(offsetof(RndGpuStatsMgr::StatBlock, mTotal) == 32);
static_assert(sizeof(RndGpuStatsMgr::StatBlock) == 384);
static_assert(offsetof(RndGpuStatsMgr, mTotalBlock) == 32);
static_assert(offsetof(RndGpuStatsMgr, mStatBlocks) == 40);
static_assert(offsetof(RndGpuStatsMgr, mNextKey) == 72);
static_assert(offsetof(RndGpuStatsMgr, mFrameSlot) == 80);
static_assert(offsetof(RndGpuStatsMgr, mResolvedFrame) == 96);
static_assert(offsetof(RndGpuStatsMgr, mBackend) == 104);
static_assert(offsetof(RndGpuStatsMgr, mCritSec) == 112);
static_assert(sizeof(RndGpuStatsMgr) == 128);
