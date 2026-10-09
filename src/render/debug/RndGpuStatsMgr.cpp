#include "render/debug/RndGpuStatsMgr.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iterator>

#include <strings.h>

#include "render/context/RndContext.h"
#include "os/system/System.h"
#include "render/system/RndDevice.h"
#include "utl/data/DataArray.h"
#include "utl/profiling/BudgetCategories.h"

namespace {

using Stat = RndGpuStatsMgr::Stat;
using StatBlock = RndGpuStatsMgr::StatBlock;

// The sorted vectors compare Symbols by the address of their text.
bool FullNameLess(const Stat* stat, const char* key) {
    return stat->mFullNameSym.Str() < key;
}

bool NameLess(const StatBlock* block, const char* key) {
    return block->mTotal.mName.Str() < key;
}

}  // namespace

template <typename T>
void RndGpuStatsMgr::Array<T>::Insert(T* position, T value) {
    if (mEnd != mCapacity) {
        std::memmove(
            position + 1,
            position,
            static_cast<std::size_t>(mEnd - position) * sizeof(T));
        *position = value;
        ++mEnd;
        return;
    }

    const auto size = mBegin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(mEnd - mBegin);
    const auto index = mBegin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(position - mBegin);
    const auto capacity = size == 0 ? std::size_t{1} : size * 2;
    auto* storage = static_cast<T*>(
        HmxAllocator::gStlAllocator.allocate(capacity * sizeof(T)));

    if (index != 0) {
        std::memmove(storage, mBegin, index * sizeof(T));
    }
    storage[index] = value;
    if (index != size) {
        std::memmove(
            storage + index + 1,
            position,
            (size - index) * sizeof(T));
    }

    if (mBegin != nullptr) {
        Free();
    }

    mBegin = storage;
    mEnd = storage + size + 1;
    mCapacity = storage + capacity;
}

void RndGpuStatsMgr::Frame::Reset() {
    mQueryCount = 0;
    mSeconds = 0.0F;
    std::fill(std::begin(mCounters), std::end(mCounters), 0UL);
}

namespace {

// The accessors report milliseconds.
constexpr float kMsPerSecond = 1000.0F;

}  // namespace

// Reconstructed from eboot.elf at 0x6F2930.
RndGpuStatsMgr::Stat::Stat(Symbol name, Stat* parent)
    : PerfTimerBase(name), mQueryKeys{}, mFrames{}, mFullNameSym() {
    mParent = parent;
    mAmbiguousParent = false;
    UpdateFullName(kSortName);
    mFullNameSym = Symbol(mFullName.c_str());
    const DataArray* config = SystemConfig(Symbol("gpu_timer"))->FindArray(name, false);
    if (config != nullptr) {
        LoadConfig(config);
    }
}

// Reconstructed from eboot.elf at 0x62D270; the deleting destructor is at
// 0x62D320. Also inlined into the manager destructor at 0x62ABA0, which
// destroys each StatBlock total in place.
RndGpuStatsMgr::Stat::~Stat() {}

// Reconstructed from eboot.elf at 0x62D3D0.
int RndGpuStatsMgr::Stat::_GetCount(unsigned long frame) const {
    return mFrames[frame].mQueryCount;
}

// Reconstructed from eboot.elf at 0x62D3E0.
float RndGpuStatsMgr::Stat::_GetAverageCount(unsigned long frame) const {
    return mFrames[frame].mLastSeconds;
}

// Reconstructed from eboot.elf at 0x62D400.
float RndGpuStatsMgr::Stat::_GetMs(unsigned long frame) const {
    return mFrames[frame].mAverageQueryCount * kMsPerSecond;
}

// Reconstructed from eboot.elf at 0x62D420.
float RndGpuStatsMgr::Stat::_GetAverageMs(unsigned long frame) const {
    return mFrames[frame].mAverageSeconds * kMsPerSecond;
}

// Reconstructed from eboot.elf at 0x62D440.
float RndGpuStatsMgr::Stat::_GetWorstMs(unsigned long frame) const {
    return mFrames[frame].mWorstSeconds * kMsPerSecond;
}

// Inlined into Init and _FindOrCreateStat.
RndGpuStatsMgr::StatBlock::StatBlock(Symbol name, Stat* parent)
    : mChildren{}, mTotal(name, parent) {}

// Reconstructed from eboot.elf at 0x62AAE0.
RndGpuStatsMgr::RndGpuStatsMgr()
    : mStats{},
      mTotalBlock(nullptr),
      mStatBlocks{},
      mNextKey(0),
      mFrameSlot(0),
      mSplitFrameTiming(false),
      mResolvedFrame(0),
      mEnableCount(0) {}

// Reconstructed from eboot.elf at 0x62ABA0. The CritSec and the two vectors
// are destroyed after the body.
RndGpuStatsMgr::~RndGpuStatsMgr() {
    for (auto** stat = mStats.mBegin; stat != mStats.mEnd; ++stat) {
        delete *stat;
    }
    mStats.mEnd = mStats.mBegin;

    for (auto** block = mStatBlocks.mBegin; block != mStatBlocks.mEnd;
         ++block) {
        delete *block;
    }
    mStatBlocks.mEnd = mStatBlocks.mBegin;
}

// Reconstructed from eboot.elf at 0x62ACB0.
void RndGpuStatsMgr::Init() {
    auto* total = new StatBlock(Symbol("GPU Total"), nullptr);
    mTotalBlock = total;
    mStatBlocks.PushBack(total);

    const auto count = NumBudgetCategories();
    for (unsigned long index = 0; index < count; ++index) {
        const auto category = static_cast<int>(index);
        auto* block = new StatBlock(BudgetCategoryName(category), &total->mTotal);
        total->mTotal.mHasChildren = true;
        block->mTotal.mBudget = BudgetCategoryGpuBudget(category);
        block->mTotal.mBudgetCategory = static_cast<unsigned int>(category);
        mStatBlocks.PushBack(block);
    }

    std::sort(
        mStatBlocks.mBegin,
        mStatBlocks.mEnd,
        [](const StatBlock* left, const StatBlock* right) {
            return left->mTotal.mName.Str() < right->mTotal.mName.Str();
        });
}

// Reconstructed from eboot.elf at 0x62AF80.
long RndGpuStatsMgr::BeginStatBlock(RndContext& context, const char* name) {
    if (mEnableCount == 0) {
        return -1;
    }

    auto* scope = context.LastGpuStatScope();
    auto* parent = scope == nullptr ? nullptr : static_cast<Stat*>(scope->mStat);
    const char* fullName = name;
    char nestedName[4096]{};
    if (parent != nullptr) {
        parent->mHasChildren = true;
        std::snprintf(
            nestedName,
            sizeof(nestedName),
            "%s %s",
            parent->mFullName.c_str(),
            name);
        fullName = nestedName;
    }

    mCritSec.Enter();
    auto* stat = _FindOrCreateStat(name, fullName, parent);
    const auto key = mNextKey++;
    stat->mQueryKeys[mFrameSlot].PushBack(key);
    mCritSec.Exit();

    context._BeginGpuStatsImpl(key);
    context.PushGpuStatScope({stat, key});
    return static_cast<long>(key);
}

// Reconstructed from eboot.elf at 0x62B2C0.
RndGpuStatsMgr::Stat* RndGpuStatsMgr::_FindOrCreateStat(
    const char* name,
    const char* fullName,
    Stat* parent) {
    const Symbol nameSym{name};
    const Symbol fullNameSym{fullName};

    mCritSec.Enter();

    auto** statPos = mStats.mBegin;
    if (mStats.mBegin != mStats.mEnd) {
        statPos = std::lower_bound(
            mStats.mBegin, mStats.mEnd, fullNameSym.Str(), FullNameLess);
    }

    if (statPos != mStats.mEnd && (*statPos)->mFullNameSym == fullNameSym) {
        auto* stat = *statPos;
        mCritSec.Exit();
        return stat;
    }

    auto* stat = new Stat(nameSym, parent);
    mStats.Insert(statPos, stat);

    auto** blockPos = mStatBlocks.mBegin;
    if (mStatBlocks.mBegin != mStatBlocks.mEnd) {
        blockPos = std::lower_bound(
            mStatBlocks.mBegin, mStatBlocks.mEnd, nameSym.Str(), NameLess);
    }

    StatBlock* block = nullptr;
    if (blockPos != mStatBlocks.mEnd && (*blockPos)->mTotal.mName == nameSym) {
        block = *blockPos;
        if (block->mTotal.mBudgetCategory != ~0U && stat->mBudget == 0.0F) {
            stat->mBudget = block->mTotal.mBudget;
            stat->mBudgetCategory = block->mTotal.mBudgetCategory;
        }
    } else {
        block = new StatBlock(nameSym, &mTotalBlock->mTotal);
        mTotalBlock->mTotal.mHasChildren = true;
        mStatBlocks.Insert(blockPos, block);
    }

    block->mChildren.PushBack(stat);

    mCritSec.Exit();
    return stat;
}

// Name not in the reference map.
RndGpuStatsMgr::Stat* RndGpuStatsMgr::_FindStat(Symbol fullName) {
    auto** pos = std::lower_bound(
        mStats.mBegin, mStats.mEnd, fullName.Str(), FullNameLess);
    if (pos == mStats.mEnd || (*pos)->mFullNameSym != fullName) {
        return nullptr;
    }
    return *pos;
}

// Reconstructed from eboot.elf at 0x62B5B0.
void RndGpuStatsMgr::EndStatBlock(RndContext& context, long key) {
    if (key < 0 || mEnableCount == 0) {
        return;
    }

    context.PopGpuStatScope();
    context._EndGpuStatsImpl(static_cast<unsigned long>(key));
}

// Reconstructed from eboot.elf at 0x62B960.
void RndGpuStatsMgr::EndFrame() {
    _GatherStats();
    _NextFrame();
}

// Reconstructed from eboot.elf at 0x62C0E0. Also inlined into EndFrame.
// Advances to the next frame slot and clears the query keys recorded there
// four frames ago.
void RndGpuStatsMgr::_NextFrame() {
    mCritSec.Enter();
    mFrameSlot = (static_cast<unsigned char>(mFrameSlot) + 1U) & 3U;
    for (auto** stat = mStats.mBegin; stat != mStats.mEnd; ++stat) {
        auto& keys = (*stat)->mQueryKeys[mFrameSlot];
        keys.mEnd = keys.mBegin;
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x62B9E0. Resolves the oldest frame slot's
// queries, derives the "GPU Total {Remainder}" statistic, smooths every
// statistic over at most 50 frames, and sums each StatBlock's statistics.
void RndGpuStatsMgr::_GatherStats() {
    auto& device = *TheRndDevice();
    if (device.mBeginFramePending) {
        device._FlushPendingBeginFrame();
    }
    auto& context = *device.mImmediateContext;
    const auto frame = static_cast<std::size_t>(mResolvedFrame);

    const auto oldestSlot = static_cast<std::size_t>(
        (static_cast<unsigned char>(mFrameSlot) + 1U) & 3U);
    for (auto** item = mStats.mBegin; item != mStats.mEnd; ++item) {
        auto& stat = **item;
        auto& timing = stat.mFrames[frame];
        timing.Reset();
        const auto& keys = stat.mQueryKeys[oldestSlot];
        for (auto* key = keys.mBegin; key != keys.mEnd; ++key) {
            const auto sample = context._EvalAndRetireGpuStatsImpl(*key);
            ++timing.mQueryCount;
            timing.mSeconds += sample.mSeconds;
            for (std::size_t i = 0; i < 6; ++i) {
                timing.mCounters[i] += sample.mCounters[i];
            }
        }
    }

    if (mEnableCount != 0) {
        static const Symbol totalName("GPU Total");

        mCritSec.Enter();
        auto* total = _FindStat(totalName);
        mCritSec.Exit();
        if (total != nullptr) {
            auto* remainder = _FindOrCreateStat(
                "{Remainder}", "GPU Total {Remainder}", total);
            const auto& totalTiming = total->mFrames[frame];
            auto& timing = remainder->mFrames[frame];
            timing.mQueryCount = 1;
            timing.mSeconds = totalTiming.mSeconds;
            std::copy(
                std::begin(totalTiming.mCounters),
                std::end(totalTiming.mCounters),
                timing.mCounters);

            for (auto** item = mStats.mBegin; item != mStats.mEnd; ++item) {
                auto& child = **item;
                if (&child == remainder || child.mParent != total) {
                    continue;
                }
                const auto& childTiming = child.mFrames[frame];
                timing.mSeconds -= childTiming.mSeconds;
                for (std::size_t i = 0; i < 6; ++i) {
                    timing.mCounters[i] -= childTiming.mCounters[i];
                }
            }
        }
    }

    constexpr int kMaxWindow = 50;
    for (auto** item = mStats.mBegin; item != mStats.mEnd; ++item) {
        auto& timing = (*item)->mFrames[frame];
        ++timing.mSampleCount;
        const auto window = std::min(timing.mSampleCount, kMaxWindow);
        const auto previousWeight = static_cast<float>(window - 1);
        const auto reciprocal = 1.0F / static_cast<float>(window);
        timing.mAverageSeconds =
            (previousWeight * timing.mAverageSeconds + timing.mSeconds) *
            reciprocal;
        timing.mAverageQueryCount =
            (previousWeight * timing.mAverageQueryCount +
             static_cast<float>(timing.mQueryCount)) *
            reciprocal;
        timing.mWorstSeconds = std::max(timing.mSeconds, timing.mWorstSeconds);
        timing.mLastSeconds = timing.mSeconds;
    }

    for (auto** item = mStatBlocks.mBegin; item != mStatBlocks.mEnd; ++item) {
        auto& block = **item;
        auto& timing = block.mTotal.mFrames[frame];
        timing.Reset();
        timing.mAverageSeconds = 0.0F;
        timing.mSampleCount = 0;
        for (auto** child = block.mChildren.mBegin;
             child != block.mChildren.mEnd;
             ++child) {
            const auto& childTiming = (*child)->mFrames[frame];
            timing.mQueryCount += childTiming.mQueryCount;
            timing.mSeconds += childTiming.mSeconds;
            timing.mAverageSeconds += childTiming.mAverageSeconds;
            timing.mSampleCount =
                std::max(timing.mSampleCount, childTiming.mSampleCount);
            for (std::size_t i = 0; i < 6; ++i) {
                timing.mCounters[i] += childTiming.mCounters[i];
            }
        }
        timing.mAverageQueryCount = timing.mSeconds;
        timing.mWorstSeconds = std::max(timing.mSeconds, timing.mWorstSeconds);
    }
}

namespace {

// The timers sort by their full names without regard to case. The binary
// sorts with EASTL's introsort.
bool FullNameCaseLess(const PerfTimerBase* a, const PerfTimerBase* b) {
    return strcasecmp(a->mFullName.c_str(), b->mFullName.c_str()) < 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x62C150. Display mode 0 lists every
// statistic, 1 every per-name total, and 2 the totals of budget categories
// and "GPU Total".
void RndGpuStatsMgr::GatherTimers(
    unsigned int displayMode,
    int sortMode,
    eastl::vector<PerfTimerBase*>& timers) {
    static_cast<void>(sortMode);
    mCritSec.Enter();
    if (displayMode == 0) {
        for (auto** stat = mStats.mBegin; stat != mStats.mEnd; ++stat) {
            timers.push_back(*stat);
        }
    } else {
        for (auto** item = mStatBlocks.mBegin; item != mStatBlocks.mEnd; ++item) {
            StatBlock* block = *item;
            if (displayMode == 2 && block->mTotal.mBudgetCategory == ~0U &&
                block != mTotalBlock) {
                continue;
            }
            timers.push_back(&block->mTotal);
        }
    }
    std::sort(timers.begin(), timers.end(), FullNameCaseLess);
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x62C610.
float RndGpuStatsMgr::GetBudget(Symbol name) {
    static_cast<void>(name);
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x62C710. The blocks are found by binary
// search on the name's Symbol address.
float RndGpuStatsMgr::GetAverageMs(Symbol name, unsigned long frame) {
    mCritSec.Enter();
    if (frame == static_cast<unsigned long>(-1)) {
        frame = mResolvedFrame;
    }
    auto** pos = std::lower_bound(
        mStatBlocks.mBegin, mStatBlocks.mEnd, name.Str(), NameLess);
    Frame timing{};
    if (pos != mStatBlocks.mEnd && (*pos)->mTotal.mName == name) {
        timing = (*pos)->mTotal.mFrames[frame];
    }
    mCritSec.Exit();
    return timing.mAverageSeconds * kMsPerSecond;
}

// Reconstructed from eboot.elf at 0x62C840.
float RndGpuStatsMgr::GetWorstMs(Symbol name, unsigned long frame) {
    mCritSec.Enter();
    if (frame == static_cast<unsigned long>(-1)) {
        frame = mResolvedFrame;
    }
    auto** pos = std::lower_bound(
        mStatBlocks.mBegin, mStatBlocks.mEnd, name.Str(), NameLess);
    Frame timing{};
    if (pos != mStatBlocks.mEnd && (*pos)->mTotal.mName == name) {
        timing = (*pos)->mTotal.mFrames[frame];
    }
    mCritSec.Exit();
    return timing.mWorstSeconds * kMsPerSecond;
}

// Reconstructed from eboot.elf at 0x62C970. A split-frame display keeps two
// history frames.
void RndGpuStatsMgr::ResetTimers() {
    mCritSec.Enter();
    const auto numFrames =
        static_cast<std::size_t>(static_cast<unsigned char>(mSplitFrameTiming)) + 1;
    for (auto** stat = mStats.mBegin; stat != mStats.mEnd; ++stat) {
        for (std::size_t i = 0; i < numFrames; ++i) {
            (*stat)->mFrames[i] = Frame{};
        }
    }
    for (auto** block = mStatBlocks.mBegin; block != mStatBlocks.mEnd; ++block) {
        for (std::size_t i = 0; i < numFrames; ++i) {
            (*block)->mTotal.mFrames[i] = Frame{};
        }
    }
    mCritSec.Exit();
}
