#include "os/profiling/PerfTimer.h"

#include <algorithm>
#include <strings.h>

#include "utl/containers/FixedVector.h"
#include "utl/data/DataArray.h"
#include "utl/text/MakeString.h"
#include "utl/time/Timer.h"

namespace {

// The deepest timer nesting UpdateFullName expects.
constexpr unsigned long kMaxDepth = 16;

}  // namespace

// At 0x19B03B0, before gTimerThresholdMs; the map's build keeps it read-only.
int PerfTimer::kWorstResetFrames = 600;

namespace {

// The weight of a new frame in the timers' running averages, at 0x19B03B4.
// Name not in the reference map.
float gTimerAverageWeight = 0.1F;

}  // namespace

float gTimerThresholdMs = 0.1F;

unsigned long PerfTimer::gCurrentFrameIndex;

// Reconstructed from eboot.elf at 0x24A730.
PerfTimer::PerfTimer(Symbol name, const DataArray* config)
    : PerfTimerBase(name), mFrames(), mEnabled(true), mRunningTimers(nullptr) {
    if (config != nullptr) {
        LoadConfig(config);
        bool enabled = mEnabled;
        config->FindData(Symbol("enabled"), enabled, false);
        mEnabled = enabled;
    }
}

// Reconstructed from eboot.elf at 0x24AA10.
void PerfTimer::UpdateMs(float ms) {
    Frame& frame = mFrames[gCurrentFrameIndex];
    frame.mMs = ms;
    if (!(ms < frame.mWorstMs)) {
        frame.mWorstMs = ms;
        frame.mWorstFrame = frame.mFrameNumber;
    }
    const float averageMs = frame.mAverageMs;
    frame.mAverageMs = averageMs == 0.0F ? ms : (ms - averageMs) * gTimerAverageWeight + averageMs;
}

// Reconstructed from eboot.elf at 0x368B20.
bool PerfTimer::Start() {
    if (!mEnabled) {
        return false;
    }
    Frame& frame = mFrames[gCurrentFrameIndex];
    if (mRunningTimers != nullptr && frame.mDepth <= 0) {
        PerfTimerBase* running = nullptr;
        PerfTimerBase* parent = nullptr;
        if (!mRunningTimers->empty()) {
            running = mRunningTimers->back();
            if (running != nullptr) {
                running->mHasChildren = true;
                if (!mIsolated && !running->mExpanded) {
                    return false;
                }
                parent = running;
            }
        }
        if (frame.mHasParent) {
            if (parent != frame.mFrameParent) {
                frame.mFrameParent = nullptr;
                frame.mParentAmbiguous = true;
            }
        } else {
            frame.mFrameParent = running;
            frame.mHasParent = true;
        }
        mRunningTimers->push_back(this);
    }
    const int depth = frame.mDepth;
    if (depth >= 0) {
        frame.mDepth = depth + 1;
        if (depth == 0) {
            frame.mStartCycles = __builtin_ia32_rdtsc();
        }
    }
    ++frame.mPendingCount;
    return true;
}

// Reconstructed from eboot.elf at 0x24A820.
void PerfTimer::EndFrame(bool reset) {
    const unsigned long index = gCurrentFrameIndex;
    Frame& frame = mFrames[index];

    const float ms = static_cast<float>(Hmx::Timer::CyclesToMs(frame.mCycles));
    frame.mMs = ms;
    if (!(ms < frame.mWorstMs)) {
        frame.mWorstMs = ms;
        frame.mWorstFrame = frame.mFrameNumber;
    }
    const float averageMs = frame.mAverageMs;
    const float weight = gTimerAverageWeight;
    frame.mAverageMs = averageMs == 0.0F ? ms : weight * (ms - averageMs) + averageMs;
    frame.mCount = frame.mPendingCount;
    frame.mAverageCount =
        (static_cast<float>(frame.mPendingCount) - frame.mAverageCount) * weight +
        frame.mAverageCount;
    frame.mPendingCount = 0;
    frame.mCycles = 0;
    frame.mDepth = 0;

    // The timer keeps a parent only when every frame that had one agrees.
    bool hadParent = false;
    if (frame.mHasParent) {
        bool ambiguous = frame.mParentAmbiguous;
        for (unsigned long other = 0; !ambiguous && other < 2; ++other) {
            const Frame& otherFrame = mFrames[other];
            if (other != index && otherFrame.mHadParent &&
                (otherFrame.mWasParentAmbiguous ||
                 otherFrame.mFrameParent != frame.mFrameParent)) {
                ambiguous = true;
            }
        }
        mParent = ambiguous ? nullptr : frame.mFrameParent;
        mAmbiguousParent = ambiguous;
        hadParent = frame.mHasParent;
    }
    frame.mHadParent = hadParent;
    frame.mHasParent = false;
    frame.mWasParentAmbiguous = frame.mParentAmbiguous;
    frame.mParentAmbiguous = false;

    const int frameNumber = frame.mFrameNumber;
    frame.mFrameNumber = frameNumber + 1;
    if (frameNumber >= frame.mWorstFrame + kWorstResetFrames) {
        frame.mWorstMs = 0.0F;
        frame.mWorstFrame = frameNumber + 1;
    }

    if (reset) {
        for (Frame& resetFrame : mFrames) {
            resetFrame.mAverageCount = 0.0F;
            resetFrame.mWorstMs = 0.0F;
            resetFrame.mAverageMs = 0.0F;
            resetFrame.mWorstFrame = frame.mFrameNumber;
        }
    }
}

// Reconstructed from eboot.elf at 0x24AD90.
float PerfTimer::_GetAverageMs(unsigned long frame) const {
    return mFrames[frame].mAverageMs;
}

// Reconstructed from eboot.elf at 0x24ADA0.
float PerfTimer::_GetWorstMs(unsigned long frame) const {
    return mFrames[frame].mWorstMs;
}

// Reconstructed from eboot.elf at 0x24ADB0. The binary sorts with
// eastl::sort's introsort and insertion sort.
void GatherSortedTimers(
    const eastl::vector<PerfTimerBase*>& source,
    eastl::vector<PerfTimerBase*>& timers,
    unsigned int displayMode,
    int sortMode) {
    static_cast<void>(displayMode);

    unsigned long count = 0;
    for (unsigned long i = 0; i < source.size(); ++i) {
        if (source[i] != nullptr) {
            ++count;
            source[i]->UpdateFullName(sortMode);
        }
    }
    timers.clear();
    timers.reserve(count);
    for (unsigned long i = 0; i < source.size(); ++i) {
        if (source[i] != nullptr) {
            timers.push_back(source[i]);
        }
    }
    std::sort(timers.begin(), timers.end(), [](PerfTimerBase* a, PerfTimerBase* b) {
        return strcasecmp(a->mFullName.c_str(), b->mFullName.c_str()) < 0;
    });
}

// Reconstructed from eboot.elf at 0x24B190.
int PerfTimer::_GetCount(unsigned long frame) const {
    return mFrames[frame].mCount;
}

// Reconstructed from eboot.elf at 0x24B1A0.
float PerfTimer::_GetAverageCount(unsigned long frame) const {
    return mFrames[frame].mAverageCount;
}

// Reconstructed from eboot.elf at 0x24B1B0.
float PerfTimer::_GetMs(unsigned long frame) const {
    return mFrames[frame].mMs;
}

// Reconstructed from eboot.elf at 0x24B720.
PerfTimerBase::PerfTimerBase(Symbol name)
    : mName(name),
      mParent(nullptr),
      mAmbiguousParent(false),
      mHasChildren(false),
      mExpanded(false),
      mIsolated(false),
      mBudget(0.0F),
      mBudgetCategory(~0U) {}

// Reconstructed from eboot.elf at 0x24B1C0. The deleting destructor is at
// 0x24B1E0.
PerfTimerBase::~PerfTimerBase() {}

// Reconstructed from eboot.elf at 0x24B760. The parents are joined by
// spaces; isolated timers and those with mAmbiguousParent set show only
// their own name.
void PerfTimerBase::UpdateFullName(int mode) {
    mFullName.erase();
    if (mIsolated) {
        mFullName << "!~ " << GetSortName(mode);
        return;
    }
    if (mAmbiguousParent) {
        mFullName << "~ " << GetSortName(mode);
        return;
    }

    FixedVector<const PerfTimerBase*, kMaxDepth> chain;
    chain.push_back(this);
    for (const auto* parent = mParent; parent != nullptr; parent = parent->mParent) {
        chain.push_back(parent);
    }
    for (unsigned long i = chain.size(); i-- != 0;) {
        if (i != chain.size() - 1) {
            mFullName << " ";
        }
        mFullName << chain[i]->GetSortName(mode);
    }
}

// Reconstructed from eboot.elf at 0x24B8C0. The values are subtracted from a
// maximum so that larger values sort first; they come from the current
// frame.
const char* PerfTimerBase::GetSortName(int mode) const {
    switch (mode) {
    case kSortName:
        return mName.Str();
    case kSortAverageMs: {
        FormatString format("%07.2f %s");
        format << 9999.0F - _GetAverageMs(0) << mName;
        return format.Str();
    }
    case kSortWorstMs: {
        FormatString format("%07.2f %s");
        format << 9999.0F - _GetWorstMs(0) << mName;
        return format.Str();
    }
    case kSortCount: {
        FormatString format("%05d %s");
        format << 99999 - _GetCount(0) << mName;
        return format.Str();
    }
    case kSortAverageCount: {
        FormatString format("%07.1f %s");
        format << 99999.0F - _GetAverageCount(0) << mName;
        return format.Str();
    }
    default:
        return Symbol().Str();
    }
}

// Reconstructed from eboot.elf at 0x24BA10. Missing keys keep the current
// values.
void PerfTimerBase::LoadConfig(const DataArray* config) {
    bool expanded = mExpanded;
    config->FindData(Symbol("expanded"), expanded, false);
    mExpanded = expanded;
    bool isolated = mIsolated;
    config->FindData(Symbol("isolated"), isolated, false);
    mIsolated = isolated;
    float budget = mBudget;
    config->FindData(Symbol("budget"), budget, false);
    mBudget = budget;
}
