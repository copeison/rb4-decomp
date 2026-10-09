#include "render/debug/overlays/RndTimersOverlay.h"

#include "os/joypads/Keyboard.h"
#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"
#include "os/system/System.h"
#include "utl/data/DataArray.h"

namespace {

// Background colors. Names not in the reference map.
const Hmx::Color sUnknown152Color(0.0F, 0.5F, 1.0F, 0.5F);  // 0x1AB1F10
const Hmx::Color sOverBudgetColor(1.0F, 0.0F, 0.0F, 0.8F);  // 0x1AB1F20

// The keyboard's arrow keys. Names not in the reference map.
constexpr int kKeyLeft = 320;
constexpr int kKeyRight = 321;
constexpr int kKeyUp = 322;
constexpr int kKeyDown = 323;

// The lines the tree shows at once. Name not in the reference map.
constexpr unsigned long kVisibleLines = 30;

}  // namespace

// Reconstructed from eboot.elf at 0x6E7850.
RndTimersOverlay::RndTimersOverlay(const char* name, unsigned int flags)
    : RndOverlayTextBase(name, flags | kFlagKeyboard | kFlagHasHelp | kFlagStripedLines),
      mThreadList(this),
      mUnknown152(false),
      mOverBudget(0.0F) {}

// Reconstructed from eboot.elf at 0x6E78C0 (deleting variant at 0x6E78F0).
RndTimersOverlay::~RndTimersOverlay() {}

// Reconstructed from eboot.elf at 0x6E7930.
void RndTimersOverlay::GetThreads(eastl::vector<ScePthread>& threads) {
    threads.clear();
    _Unknown12(threads);
}

// Reconstructed from eboot.elf at 0x6E7940. An unknown thread leaves the
// list empty.
void RndTimersOverlay::GetThreadTimers(
    ScePthread thread,
    eastl::vector<PerfTimerBase*>& timers) {
    timers.clear();
    for (auto& view : mThreadList.mThreads) {
        if (view.mThread == thread) {
            view.AppendTimers(timers);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x6E79C0.
void RndTimersOverlay::_Update() {
    mThreadList.Update();
}

// Reconstructed from eboot.elf at 0x6E79D0. New threads are sorted into
// place, and the first thread starts expanded. A thread that folds and is
// folded drops its items; the selected item survives a regather only while
// it is still listed. Without a selection the first thread is selected,
// with its first item when threads do not fold.
void RndTimersOverlay::TimedThreadListView::Update() {
    const bool wasEmpty = mThreads.empty();
    mThreadKeys.clear();
    mOwner->_Unknown12(mThreadKeys);

    bool added = false;
    for (ScePthread thread : mThreadKeys) {
        bool found = false;
        for (auto& view : mThreads) {
            if (view.mThread == thread) {
                found = true;
                break;
            }
        }
        if (!found) {
            mThreads.push_back(*new ThreadTimersListView(mOwner, thread));
            added = true;
        }
    }
    mThreadKeys.clear();
    if (added) {
        mThreads.sort(ThreadTimersListView::ThreadCmp());
    }
    if (wasEmpty && !mThreads.empty()) {
        mThreads.front().mExpanded = true;
    }

    for (auto& view : mThreads) {
        const bool selected = &view == mSelectedThread;
        TimerItemView* selectedItem = selected ? mSelectedItem : nullptr;
        bool keepSelection = true;
        const unsigned int displayMode = static_cast<unsigned int>(mDisplayMode);
        const int sortMode = mSortMode;
        if (mOwner->_Unknown11() && !view.mExpanded) {
            view._ClearTimers();
        } else {
            view._GatherTimers(selectedItem, displayMode, sortMode, &keepSelection);
            if (selected && !keepSelection) {
                mSelectedItem = nullptr;
            }
        }
    }

    if (mSelectedThread == nullptr && !mThreads.empty()) {
        mSelectedThread = &mThreads.front();
        mSelectedItem = mOwner->_Unknown11() ? nullptr : mSelectedThread->FirstItem();
    }
    if (mSelectedThread != nullptr && mSelectedItem == nullptr && !mOwner->_Unknown11()) {
        mSelectedItem = mSelectedThread->FirstItem();
    }
    _UpdateScroll();
}

// Inlined into HandleKeyboardMsg. Past the last item the next thread is
// selected; past the last thread the last item of the last thread.
void RndTimersOverlay::TimedThreadListView::SelectNextItem() {
    ThreadTimersListView* thread = mSelectedThread;
    if (thread != nullptr) {
        TimerItemView* next = mSelectedItem != nullptr
                                  ? thread->mItems.next(*mSelectedItem)
                                  : thread->FirstItem();
        if (next != nullptr) {
            mSelectedItem = next;
            return;
        }
        mSelectedItem = nullptr;
        mSelectedThread = mThreads.next(*thread);
        if (mSelectedThread != nullptr) {
            return;
        }
    }
    if (!mThreads.empty()) {
        mSelectedThread = &mThreads.back();
        mSelectedItem =
            mSelectedThread->mItems.empty() ? nullptr : &mSelectedThread->mItems.back();
    }
}

// Inlined into HandleKeyboardMsg. Above the first item a folding thread
// selects itself; otherwise the last item of the previous thread is
// selected, and above the first thread the first thread.
void RndTimersOverlay::TimedThreadListView::SelectPrevItem() {
    ThreadTimersListView* thread = mSelectedThread;
    if (thread != nullptr) {
        if (mSelectedItem != nullptr) {
            mSelectedItem = thread->mItems.prev(*mSelectedItem);
            if (mSelectedItem != nullptr || mOwner->_Unknown11()) {
                return;
            }
        }
        mSelectedThread = mThreads.prev(*thread);
        if (mSelectedThread != nullptr) {
            mSelectedItem =
                mSelectedThread->mItems.empty() ? nullptr : &mSelectedThread->mItems.back();
            return;
        }
    }
    if (!mThreads.empty()) {
        mSelectedThread = &mThreads.front();
        mSelectedItem = mOwner->_Unknown11() ? nullptr : mSelectedThread->FirstItem();
    }
}

// Inlined into HandleKeyboardMsg.
void RndTimersOverlay::TimedThreadListView::ExpandItem() {
    ThreadTimersListView* thread = mSelectedThread;
    if (thread == nullptr) {
        return;
    }
    if (TimerItemView* item = mSelectedItem) {
        if (item->mType == TimerItemView::kTypeTimer && item->mTimer->mHasChildren) {
            item->mTimer->mExpanded = true;
        }
    } else {
        thread->mExpanded = true;
    }
}

// Inlined into HandleKeyboardMsg. Folds the selected timer, or else moves
// to its parent and folds that, or else folds the thread.
void RndTimersOverlay::TimedThreadListView::CollapseItem() {
    const bool threadsFold = mOwner->_Unknown11();
    ThreadTimersListView* thread = mSelectedThread;
    if (thread == nullptr) {
        return;
    }
    TimerItemView* item = mSelectedItem;
    if (item == nullptr) {
        if (threadsFold) {
            thread->mExpanded = false;
        }
        return;
    }
    if (item->mType == TimerItemView::kTypeTimer) {
        PerfTimerBase* timer = item->mTimer;
        if (timer->mHasChildren && timer->mExpanded) {
            timer->mExpanded = false;
            return;
        }
        PerfTimerBase* parent = timer->mParent;
        if (parent != nullptr) {
            for (auto& other : thread->mItems) {
                if (other.mTimer == parent) {
                    mSelectedItem = &other;
                    if (other.mType == TimerItemView::kTypeTimer && parent->mHasChildren) {
                        parent->mExpanded = false;
                    }
                    return;
                }
            }
        }
    }
    if (threadsFold) {
        mSelectedItem = nullptr;
        thread->mExpanded = false;
    }
}

// Reconstructed from eboot.elf at 0x6E7D40.
bool RndTimersOverlay::HandleKeyboardMsg(const KeyboardKeyMsg& msg) {
    switch (msg.GetKey()) {
    case 'd':
        mThreadList.mDisplayMode = (mThreadList.mDisplayMode + 1) % 3;
        return true;
    case 'i':
        if (mThreadList.mSelectedThread != nullptr) {
            TimerItemView* item = mThreadList.mSelectedItem;
            if (item != nullptr && item->mType == TimerItemView::kTypeTimer) {
                item->mTimer->mIsolated = !item->mTimer->mIsolated;
            }
        }
        return true;
    case 's':
        mThreadList.mSortMode = (mThreadList.mSortMode + 1) % 5;
        return true;
    case kKeyLeft:
        mThreadList.CollapseItem();
        return true;
    case kKeyRight:
        mThreadList.ExpandItem();
        return true;
    case kKeyUp:
        mThreadList.SelectPrevItem();
        return true;
    case kKeyDown:
        mThreadList.SelectNextItem();
        return true;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x6E8400.
void RndTimersOverlay::PrintHelp(TextStream& stream) {
    stream << "Keyboard controls:\n"
              "  Up/Down: navigate tree control\n"
              "  Left:    collapse tree control\n"
              "  Right:   expand tree control\n"
              "  D:       cycle display type\n"
              "  S:       cycle sorting method\n";
}

// Reconstructed from eboot.elf at 0x6E8420. Runs the "timer_script" block
// of the "rnd" system configuration before each print, looked up once.
void RndTimersOverlay::_Print(TextStream& stream) {
    static DataArray* sTimerScript =
        SystemConfig(Symbol("rnd"))->FindArray(Symbol("timer_script"), false);
    if (sTimerScript != nullptr) {
        DataExecuteBlock(sTimerScript, 1);
    }
    thePerfMgr.Lock();
    mThreadList.Draw(stream);
    thePerfMgr.Unlock();
}

// Reconstructed from eboot.elf at 0x6E8740. Blends every channel towards
// red by the over-budget amount.
Hmx::Color RndTimersOverlay::_GetBackgroundColor() const {
    float amount = mOverBudget;
    Hmx::Color color;
    if (mUnknown152) {
        amount *= 0.5F;
        color = sUnknown152Color;
    } else {
        color = RndOverlayTextBase::_GetBackgroundColor();
    }
    if (amount > 0.0F) {
        color.red += (sOverBudgetColor.red - color.red) * amount;
        color.green += (sOverBudgetColor.green - color.green) * amount;
        color.blue += (sOverBudgetColor.blue - color.blue) * amount;
        color.alpha += (sOverBudgetColor.alpha - color.alpha) * amount;
    }
    return color;
}

// Reconstructed from eboot.elf at 0x6EA8B0. Items without a timer are
// skipped.
void RndTimersOverlay::ThreadTimersListView::AppendTimers(
    eastl::vector<PerfTimerBase*>& timers) {
    timers.reserve(timers.size() + mItems.size());
    for (auto& item : mItems) {
        if (item.mTimer != nullptr) {
            timers.push_back(item.mTimer);
        }
    }
}

// Reconstructed from eboot.elf at 0x6EAB30.
RndTimersOverlay::TimedThreadListView::~TimedThreadListView() {
    while (!mThreads.empty()) {
        delete &mThreads.front();
    }
}

// Inlined into _UpdateScroll. A folded thread takes its own line.
unsigned long RndTimersOverlay::TimedThreadListView::_NumLines(
    ThreadTimersListView& view) const {
    if (mOwner->_Unknown11() && !view.mExpanded) {
        return 1;
    }
    return 1 + view.mItems.size();
}

// Reconstructed from eboot.elf at 0x6EAC60. A tree that fits never
// scrolls; otherwise the selected line is kept two lines inside the
// visible ones.
void RndTimersOverlay::TimedThreadListView::_UpdateScroll() {
    if (mSelectedThread == nullptr || mThreads.empty()) {
        mScroll = 0;
        return;
    }
    unsigned long total = 0;
    for (auto& view : mThreads) {
        total += _NumLines(view);
    }
    if (total <= kVisibleLines) {
        mScroll = 0;
        return;
    }

    unsigned long line = 0;
    for (auto& view : mThreads) {
        if (&view != mSelectedThread) {
            line += _NumLines(view);
            continue;
        }
        const bool threadsFold = mOwner->_Unknown11();
        if (!threadsFold || mSelectedItem != nullptr) {
            line += threadsFold ? 1 : 0;
            if (!mOwner->_Unknown11() || view.mExpanded) {
                ++line;
                for (auto& item : view.mItems) {
                    if (&item == mSelectedItem) {
                        break;
                    }
                    ++line;
                }
            }
        }
        break;
    }

    if (line + 2 >= mScroll + kVisibleLines) {
        mScroll = line - (kVisibleLines - 2);
    } else {
        const unsigned long top = line > 2 ? line - 2 : 0;
        if (top < mScroll) {
            mScroll = top;
        }
    }
}
