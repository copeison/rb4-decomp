#include "os/profiling/PerfTimer.h"

#include "utl/containers/FixedVector.h"
#include "utl/data/DataArray.h"
#include "utl/text/MakeString.h"

namespace {

// The deepest timer nesting UpdateFullName expects.
constexpr unsigned long kMaxDepth = 16;

}  // namespace

float gTimerThresholdMs = 0.1F;

// Reconstructed from eboot.elf at 0x24B720.
PerfTimerBase::PerfTimerBase(Symbol name)
    : mName(name),
      mParent(nullptr),
      mUnknown40(false),
      mHasChildren(false),
      mExpanded(false),
      mIsolated(false),
      mBudget(0.0F),
      mBudgetCategory(~0U) {}

// Reconstructed from eboot.elf at 0x24B1C0. The deleting destructor is at
// 0x24B1E0.
PerfTimerBase::~PerfTimerBase() {}

// Reconstructed from eboot.elf at 0x24B760. The parents are joined by
// spaces; isolated timers and those with mUnknown40 set show only their own
// name.
void PerfTimerBase::UpdateFullName(int mode) {
    mFullName.erase();
    if (mIsolated) {
        mFullName << "!~ " << GetSortName(mode);
        return;
    }
    if (mUnknown40) {
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
