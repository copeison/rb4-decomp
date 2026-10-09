#include "render/debug/overlays/RndOverlayGraphBase.h"

#include "os/profiling/PerfTimer.h"
#include "render/debug/RndOverlayMgr.h"
#include "utl/time/TimeMgr.h"

namespace {

// The budget line's color, built at startup. Name not in the reference
// map.
const Hmx::Color sBudgetColor(0.75F, 0.75F, 0.75F, 1.0F);  // 0x1AB1E30

// The current real time in seconds. Inlined. Name not in the reference
// map.
float RealTimeSeconds() {
    return TheTimeMgr->mRealTime.SplitMs() * 0.001F;
}

// Reconstructed from eboot.elf at 0x6E6B70. Drops the timer of the overlay
// drawing itself. Its first argument is unused and dropped here. Name not
// in the reference map.
void RemoveOverlayTimers(eastl::vector<PerfTimerBase*>& timers) {
    static Symbol sDrawOverlays;
    if (sDrawOverlays == Symbol()) {
        sDrawOverlays = Symbol("draw_overlays");
    }
    for (unsigned long i = timers.size(); i-- != 0;) {
        if (timers[i]->mName == sDrawOverlays) {
            timers.erase(timers.begin() + i);
        }
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6E6280. Without a budget the graph
// tops out at a 30 fps frame. The ten series colors are built on first
// use.
RndTimerGraphOverlay::RndTimerGraphOverlay(
    const char* name,
    unsigned int flags,
    const char* timersName,
    float budget)
    : RndOverlayGraphBase(name, flags),
      mTimersOverlay(
          static_cast<RndTimersOverlay*>(RndOverlayMgr::GetOverlay(Symbol(timersName)))),
      mTimeWindow(5.0F) {
    mMaxMs = budget > 0.0F ? budget + budget : 1000.0F / 30.0F;
    mTimers.reserve(20);
    if (budget > 0.0F) {
        mBudgetLine.resize(2);
        mBudgetLine[0].y = budget;
        mBudgetLine[1].y = budget;
    }

    static const Hmx::Color sPalette[] = {
        Hmx::Color::GetGreen(),
        Hmx::Color::GetCyan(),
        Hmx::Color::GetBlue(),
        Hmx::Color::GetViolet(),
        Hmx::Color::GetYellow(),
        Hmx::Color::GetOrange(),
        Hmx::Color::GetRed(),
        Hmx::Color::GetMagenta(),
        Hmx::Color::GetGrey(),
        Hmx::Color(1.0F, 0.5F, 0.5F, 1.0F),
    };
    mPalette.resize(sizeof(sPalette) / sizeof(sPalette[0]));
    for (unsigned long i = 0; i < mPalette.size(); ++i) {
        mPalette[i].mColor = sPalette[i];
        mPalette[i].mUseCount = 0;
    }
}

// Reconstructed from eboot.elf at 0x6E6880 (deleting variant at 0x6E6930).
RndTimerGraphOverlay::~RndTimerGraphOverlay() {}

// Reconstructed from eboot.elf at 0x6E6950. The budget line spans the
// time window.
void RndTimerGraphOverlay::_Update() {
    mTimersOverlay->Update();
    const float now = RealTimeSeconds();
    if (mBudgetLine.size() != 0) {
        mBudgetLine[0].x = now - mTimeWindow;
        mBudgetLine[1].x = now;
    }
    mTimersOverlay->GetThreads(mThreads);
    mThreadSeries.reserve(mThreads.size());
    for (ScePthread thread : mThreads) {
        mTimersOverlay->GetThreadTimers(thread, mTimers);
        RemoveOverlayTimers(mTimers);
        if (!mTimers.empty()) {
            _AddSamples(*_FindOrAddThread(thread), mTimers, now);
        }
    }
    for (ThreadSeries* thread : mThreadSeries) {
        _TrimSamples(*thread, now);
    }
    mThreads.clear();
    mTimers.clear();
}

// Reconstructed from eboot.elf at 0x6E6C80.
RndTimerGraphOverlay::ThreadSeries* RndTimerGraphOverlay::_FindOrAddThread(ScePthread thread) {
    for (ThreadSeries* series : mThreadSeries) {
        if (series->mThread == thread) {
            return series;
        }
    }
    auto* series = new ThreadSeries;
    series->mThread = thread;
    mThreadSeries.push_back(series);
    return series;
}

// Reconstructed from eboot.elf at 0x6E6DD0. A new series reserves room for
// 200 samples.
void RndTimerGraphOverlay::_AddSamples(
    ThreadSeries& thread,
    const eastl::vector<PerfTimerBase*>& timers,
    float now) {
    for (PerfTimerBase* timer : timers) {
        TimerSeries* series = nullptr;
        for (TimerSeries* existing : thread.mSeries) {
            if (existing->mTimer == timer) {
                series = existing;
                break;
            }
        }
        if (series == nullptr) {
            series = new TimerSeries;
            series->mPoints.reserve(200);
            series->mTimer = timer;
            PaletteEntry* least = mPalette.begin();
            for (PaletteEntry* entry = least + 1; entry < mPalette.end(); ++entry) {
                if (entry->mUseCount < least->mUseCount) {
                    least = entry;
                }
            }
            ++least->mUseCount;
            series->mColor = least->mColor;
            thread.mSeries.push_back(series);
        }
        const Vector2 sample = {now, timer->_GetMs(0)};
        series->mPoints.push_back(sample);
    }
}

// Reconstructed from eboot.elf at 0x6E7140. A deleted series returns its
// palette color.
void RndTimerGraphOverlay::_TrimSamples(ThreadSeries& thread, float now) {
    const float start = now - mTimeWindow;
    for (unsigned long i = thread.mSeries.size(); i-- != 0;) {
        TimerSeries* series = thread.mSeries[i];
        unsigned long first = 0;
        while (first < series->mPoints.size() && start > series->mPoints[first].x) {
            ++first;
        }
        if (first < series->mPoints.size()) {
            if (first >= 2) {
                series->mPoints.erase(series->mPoints.begin(), series->mPoints.begin() + first - 1);
            }
            continue;
        }
        for (PaletteEntry& entry : mPalette) {
            const Hmx::Color& color = series->mColor;
            if (entry.mColor.red == color.red && entry.mColor.green == color.green &&
                entry.mColor.blue == color.blue && entry.mColor.alpha == color.alpha) {
                --entry.mUseCount;
                break;
            }
        }
        delete series;
        thread.mSeries.erase(thread.mSeries.begin() + i);
    }
}

// Reconstructed from eboot.elf at 0x6E72F0.
RndOverlayGraphBase::GraphOptions RndTimerGraphOverlay::_GetOptions() {
    GraphOptions options;
    options.mShowLegend = true;
    options.mUnknown8 = 0;
    return options;
}

// Reconstructed from eboot.elf at 0x6E7340. The time axis ends now; the
// value axis reaches the graph's top.
RndOverlayGraphBase::GraphAxes RndTimerGraphOverlay::_GetAxes() {
    GraphAxes axes;
    const float now = RealTimeSeconds();
    axes.mX.mLabel = Symbol("Time (sec)");
    axes.mX.mMax = now;
    axes.mX.mMin = now - mTimeWindow;
    axes.mX.mStep = 1.0F;
    axes.mY.mLabel = Symbol("Timers (ms)");
    axes.mY.mMin = 0.0F;
    axes.mY.mMax = mMaxMs;
    axes.mY.mStep = 1.0F;
    axes.mY.mUnknown20 = 1.0F;
    axes.mUnknown64 = now;
    axes.mUnknown68 = 0;
    return axes;
}

// Reconstructed from eboot.elf at 0x6E7470.
unsigned long RndTimerGraphOverlay::_GetNumSeries() {
    unsigned long count = 0;
    for (ThreadSeries* thread : mThreadSeries) {
        count += thread->mSeries.size();
    }
    return count + (mBudgetLine.size() != 0 ? 1 : 0);
}

// Reconstructed from eboot.elf at 0x6E74C0. The timer series follow in
// thread order.
RndOverlayGraphBase::GraphSeries RndTimerGraphOverlay::_GetSeries(unsigned long index) {
    if (mBudgetLine.size() != 0) {
        if (index == 0) {
            GraphSeries series;
            series.mName = Symbol("budget");
            series.mColor = sBudgetColor;
            series.mPoints = mBudgetLine.begin();
            series.mNumPoints = mBudgetLine.size();
            return series;
        }
        --index;
    }
    TimerSeries* found = nullptr;
    for (ThreadSeries* thread : mThreadSeries) {
        const unsigned long count = thread->mSeries.size();
        if (index < count) {
            found = thread->mSeries[index];
            break;
        }
        index -= count;
    }
    GraphSeries series;
    series.mName = found->mTimer->mName;
    series.mColor = found->mColor;
    series.mPoints = found->mPoints.begin();
    series.mNumPoints = found->mPoints.size();
    return series;
}
