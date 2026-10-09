#include "render/debug/overlays/RndTimersOverlay.h"

#include <cstring>

#include "os/joypads/Keyboard.h"
#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"
#include "os/system/System.h"
#include "utl/data/DataArray.h"
#include "utl/text/MakeString.h"
#include "utl/threading/PollMgr.h"
#include "utl/threading/Thread.h"

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

// How much the over-budget tint fades per print. Name not in the reference
// map.
constexpr float kOverBudgetDecay = 0.05F;

}  // namespace

bool RndTimersOverlay::gSplitFrameTiming = false;

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
        view._UpdateTimers(
            selectedItem, static_cast<unsigned int>(mDisplayMode), mSortMode, &keepSelection);
        if (selected && !keepSelection) {
            mSelectedItem = nullptr;
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
        if (TimerItemView* parentItem = thread->_FindParentItem(*item)) {
            mSelectedItem = parentItem;
            PerfTimerBase* parent = parentItem->mTimer;
            if (parentItem->mType == TimerItemView::kTypeTimer && parent->mHasChildren) {
                parent->mExpanded = false;
            }
            return;
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

// Reconstructed from eboot.elf at 0x6E8560. Printing stops after the
// visible lines.
void RndTimersOverlay::TimedThreadListView::Draw(TextStream& stream) {
    stream << "[ Displaying: ";
    switch (mDisplayMode) {
    case 0:
        stream << "hierarchical";
        break;
    case 1:
        stream << "flat list";
        break;
    case 2:
        stream << "budget timers";
        break;
    default:
        break;
    }
    stream << " | Sorting by: ";
    switch (mSortMode) {
    case PerfTimerBase::kSortName:
        stream << "name";
        break;
    case PerfTimerBase::kSortAverageMs:
        stream << "avg ms";
        break;
    case PerfTimerBase::kSortWorstMs:
        stream << "worst ms";
        break;
    case PerfTimerBase::kSortCount:
        stream << "call count";
        break;
    case PerfTimerBase::kSortAverageCount:
        stream << "avg call count";
        break;
    default:
        break;
    }
    stream << " | Threshold: worst >= " << MakeString("%.2f", gTimerThresholdMs) << "ms";
    stream << " ]\n";

    LineRange lines = {mScroll, mScroll + kVisibleLines, 0};
    for (auto& view : mThreads) {
        const bool selected = &view == mSelectedThread;
        view.Draw(stream, selected, selected ? mSelectedItem : nullptr, lines);
        if (lines.mLine >= lines.mEnd) {
            break;
        }
    }
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

// Reconstructed from eboot.elf at 0x6E88E0. A timer is indented once per
// parent up to the first isolated one, and once more when it or a parent is
// isolated or has mUnknown40 set. The name is the full sort name while the
// "show_timer_sort_names" script variable is set.
void RndTimersOverlay::TimerItemView::_UpdateText() {
    switch (mType) {
    case kTypeTimer: {
        mText.erase();
        const PerfTimerBase* timer = mTimer;
        bool indent = timer->mUnknown40;
        bool isolated = timer->mIsolated;
        if (!isolated) {
            for (const PerfTimerBase* parent = timer->mParent; parent != nullptr;
                 parent = parent->mParent) {
                mText << "  ";
                indent = indent || parent->mUnknown40;
                isolated = parent->mIsolated;
                if (isolated) {
                    break;
                }
            }
        }
        if (indent || isolated) {
            mText << "  ";
        }
        mText << (!timer->mHasChildren ? " " : timer->mExpanded ? "-" : "+");
        static const unsigned long sShowSortNames =
            DataVarIndex(Symbol("show_timer_sort_names"), DataNode(0));
        if (DataVariable(sShowSortNames).mValue.integer != 0) {
            mText << mTimer->mFullName.c_str();
        } else {
            mText << mTimer->mName.Str();
        }
        break;
    }
    case kTypeIsolatedHeader:
        mText = " <isolated timers>";
        break;
    case kTypeAmbiguousHeader:
        mText = " <ambiguous parents>";
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x6E8BF0.
RndTimersOverlay::TimerItemView* RndTimersOverlay::TimerItemView::_NewHeader(
    RndTimersOverlay* owner,
    Type type) {
    return new TimerItemView(owner, nullptr, type);
}

// Reconstructed from eboot.elf at 0x6E8C80. The halves are numbered "(0)"
// and "(1)".
void RndTimersOverlay::TimerItemView::_PrintHeader(
    RndTimersOverlay* overlay,
    TextStream& stream,
    const char* name,
    unsigned long width,
    bool split) {
    static_cast<void>(overlay);
    split = split && gSplitFrameTiming;
    const unsigned long numHalves = split ? 2 : 1;
    for (unsigned long half = 0; half < numHalves; ++half) {
        StackString<32> header(name);
        if (split) {
            header << " (" << half << ")";
        }
        stream.Print(header.c_str());
        for (unsigned long length = std::strlen(header.c_str()); length < width; ++length) {
            stream << '_';
        }
    }
}

// Reconstructed from eboot.elf at 0x6E8EC0.
void RndTimersOverlay::TimerItemView::_PrintPadded(
    RndTimersOverlay* overlay,
    TextStream& stream,
    const char* text,
    unsigned long width) {
    static_cast<void>(overlay);
    stream << text;
    for (unsigned long length = std::strlen(text); length < width; ++length) {
        stream << ' ';
    }
}

// Reconstructed from eboot.elf at 0x6E8FD0.
void RndTimersOverlay::TimerItemView::_PrintStat(
    RndTimersOverlay* overlay,
    TextStream& stream,
    int value,
    unsigned long width) {
    char buffer[32] = {};
    _PrintPadded(overlay, stream, PrintIntWithCommas(value, buffer, sizeof(buffer)), width);
}

// Inlined into Draw.
inline void RndTimersOverlay::TimerItemView::_PrintTimer(TextStream& stream, float ms) {
    _PrintPadded(mOwner, stream, MakeString("%.2fms", ms), 12);
}

// Reconstructed from eboot.elf at 0x6E9120. The name column is one
// character narrower to make room for the underscores.
void RndTimersOverlay::TimerItemView::DrawHeader(
    RndTimersOverlay* overlay,
    TextStream& stream,
    const char* prefix,
    unsigned long width) {
    stream << prefix << "___";
    _PrintHeader(overlay, stream, "timer", width - 1, false);
    _PrintHeader(overlay, stream, "dt", 12, true);
    _PrintHeader(overlay, stream, "avg", 12, true);
    _PrintHeader(overlay, stream, "worst", 12, true);
    _PrintHeader(overlay, stream, "count", 8, true);
    _PrintHeader(overlay, stream, "avg count", 10, true);
    _PrintHeader(overlay, stream, "budget", 12, false);
    overlay->_Unknown14(stream);
    stream << "\n";
}

// Reconstructed from eboot.elf at 0x6E9230. Each timing column lists the
// halves of a split frame in turn. A timer over budget in any half tints
// the background fully; the tint then fades.
void RndTimersOverlay::TimerItemView::Draw(
    TextStream& stream,
    const char* prefix,
    unsigned long width,
    bool selected) {
    mOwner->SetSelectedItem(selected);
    StackString<128> line;
    line << (selected ? ">" : " ") << prefix << " ";
    if (mType != kTypeTimer) {
        stream.Print(line.c_str());
        stream.Print(mText.c_str());
    } else {
        const unsigned long numFrames = gSplitFrameTiming ? 2 : 1;
        PerfTimerBase* timer = mTimer;
        const float budget = timer->mBudget;
        bool overBudget = false;
        if (budget != 0.0F) {
            for (unsigned long frame = 0; frame < numFrames; ++frame) {
                if (timer->_GetMs(frame) > budget) {
                    overBudget = true;
                }
            }
        }
        if (overBudget) {
            mOverBudget = 1.0F;
        } else {
            float amount = mOverBudget - kOverBudgetDecay;
            if (amount < 0.0F) {
                amount = 0.0F;
            }
            mOverBudget = amount > 1.0F ? 1.0F : amount;
        }
        mOwner->SetOverBudget(mOverBudget);

        stream.Print(line.c_str());
        _PrintPadded(mOwner, stream, mText.c_str(), width);
        for (unsigned long frame = 0; frame < numFrames; ++frame) {
            _PrintTimer(stream, timer->_GetMs(frame));
        }
        for (unsigned long frame = 0; frame < numFrames; ++frame) {
            _PrintTimer(stream, timer->_GetAverageMs(frame));
        }
        for (unsigned long frame = 0; frame < numFrames; ++frame) {
            _PrintTimer(stream, timer->_GetWorstMs(frame));
        }
        for (unsigned long frame = 0; frame < numFrames; ++frame) {
            _PrintStat(mOwner, stream, timer->_GetCount(frame), 8);
        }
        for (unsigned long frame = 0; frame < numFrames; ++frame) {
            _PrintPadded(mOwner, stream, MakeString("%.1f", timer->_GetAverageCount(frame)), 10);
        }
        if (budget > 0.0F) {
            _PrintTimer(stream, budget);
        } else {
            _PrintPadded(mOwner, stream, "<none>", 12);
        }
        mOwner->_Unknown15(stream, *timer, numFrames);
    }
    stream << "\n";
    mOwner->SetSelectedItem(false);
    mOwner->SetOverBudget(0.0F);
}

// Reconstructed from eboot.elf at 0x6E9A90.
bool RndTimersOverlay::ThreadTimersListView::IsBefore(
    const ThreadTimersListView& a,
    const ThreadTimersListView& b) {
    if (a.mThread == Thread::s_MainThreadID) {
        return true;
    }
    if (b.mThread == Thread::s_MainThreadID) {
        return false;
    }
    const bool aWorker = PollMgr::IsWorkerThread(a.mThread);
    const bool bWorker = PollMgr::IsWorkerThread(b.mThread);
    if (aWorker != bWorker) {
        return aWorker;
    }
    const StackString<64> aName(Thread::ThreadIdToName(a.mThread));
    const StackString<64> bName(Thread::ThreadIdToName(b.mThread));
    if (*aName.c_str() != '\0') {
        if (*bName.c_str() == '\0') {
            return true;
        }
        return strcasecmp(aName.c_str(), bName.c_str()) < 0;
    }
    if (*bName.c_str() != '\0') {
        return false;
    }
    return a.mThread < b.mThread;
}

// Reconstructed from eboot.elf at 0x6E9C90.
RndTimersOverlay::TimerItemView* RndTimersOverlay::ThreadTimersListView::_FindParentItem(
    const TimerItemView& item) {
    if (item.mType != TimerItemView::kTypeTimer) {
        return nullptr;
    }
    const PerfTimerBase* parent = item.mTimer->mParent;
    if (parent == nullptr) {
        return nullptr;
    }
    for (auto& other : mItems) {
        if (other.mTimer == parent) {
            return &other;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x6E9CE0.
void RndTimersOverlay::ThreadTimersListView::_UpdateTimers(
    TimerItemView* selected,
    unsigned int displayMode,
    int sortMode,
    bool* keepSelection) {
    if (mOwner->_Unknown11() && !mExpanded) {
        _ClearTimers();
    } else {
        _GatherTimers(selected, displayMode, sortMode, keepSelection);
    }
}

// Reconstructed from eboot.elf at 0x6E9D80. The current items are set
// aside and reused for the same timers and headers; the rest are deleted.
// The selected ambiguous-parents header is kept even without its timers.
void RndTimersOverlay::ThreadTimersListView::_GatherTimers(
    TimerItemView* selected,
    unsigned int displayMode,
    int sortMode,
    bool* keepSelection) {
    *keepSelection = selected != nullptr && selected->mType != TimerItemView::kTypeTimer;
    const unsigned long numFrames = gSplitFrameTiming ? 2 : 1;
    mOwner->_Unknown13(mThread, mTimers, displayMode, sortMode);

    ItemList oldItems;
    oldItems.splice(mItems);
    // Moves the old item of the type and timer to the end of the list, or
    // adds a new one.
    auto keepItem = [&](TimerItemView::Type type, PerfTimerBase* timer) {
        for (auto& item : oldItems) {
            if (item.mType == type && item.mTimer == timer) {
                oldItems.remove(item);
                mItems.push_back(item);
                item._UpdateText();
                return;
            }
        }
        mItems.push_back(*new TimerItemView(mOwner, timer, type));
    };

    bool headerAdded[3] = {false, false, false};
    for (PerfTimerBase* timer : mTimers) {
        if (selected != nullptr && selected->mType == TimerItemView::kTypeTimer &&
            selected->mTimer == timer) {
            *keepSelection = true;
        } else if (!timer->mIsolated) {
            float worst = 0.0F;
            for (unsigned long frame = 0; frame < numFrames; ++frame) {
                const float ms = timer->_GetWorstMs(frame);
                worst = ms > worst ? ms : worst;
            }
            if (!(worst >= gTimerThresholdMs)) {
                continue;
            }
            bool folded = false;
            for (const PerfTimerBase* parent = timer->mParent; parent != nullptr;
                 parent = parent->mParent) {
                if (!parent->mExpanded) {
                    folded = true;
                    break;
                }
            }
            if (folded) {
                continue;
            }
        }

        TimerItemView::Type header = TimerItemView::kTypeTimer;
        if (timer->mIsolated ||
            (selected != nullptr && selected->mType == TimerItemView::kTypeIsolatedHeader)) {
            header = TimerItemView::kTypeIsolatedHeader;
        } else if (timer->mUnknown40) {
            header = TimerItemView::kTypeAmbiguousHeader;
        }
        if (header != TimerItemView::kTypeTimer && !headerAdded[header]) {
            headerAdded[header] = true;
            keepItem(header, nullptr);
        }
        keepItem(TimerItemView::kTypeTimer, timer);
    }
    if (selected != nullptr && selected->mType == TimerItemView::kTypeAmbiguousHeader &&
        !headerAdded[TimerItemView::kTypeAmbiguousHeader]) {
        keepItem(TimerItemView::kTypeAmbiguousHeader, nullptr);
    }

    while (!oldItems.empty()) {
        delete &oldItems.front();
    }
    mTimers.clear();
}

// Reconstructed from eboot.elf at 0x6EA440. Only the lines from
// `lines.mFirst` print; each line counts, and printing stops at
// `lines.mEnd`. When threads fold, the thread line comes first and the
// items are indented under it.
void RndTimersOverlay::ThreadTimersListView::Draw(
    TextStream& stream,
    bool selected,
    TimerItemView* selectedItem,
    LineRange& lines) {
    StackString<32> prefix;
    if (mOwner->_Unknown11()) {
        if (lines.mLine >= lines.mFirst) {
            StackString<64> name(Thread::ThreadIdToName(mThread));
            if (*name.c_str() == '\0') {
                name = MakeString("<unknown thread, id 0x%X>", mThread);
            }
            const bool highlighted = selected && selectedItem == nullptr;
            mOwner->SetSelectedItem(highlighted);
            TextStream& line =
                stream << (highlighted ? "> " : "  ") << (mExpanded ? "-" : "+") << "Thread '";
            line.Print(name.c_str());
            line << "' Timers:\n";
            mOwner->SetSelectedItem(false);
            prefix = "   ";
        }
        if (++lines.mLine == lines.mEnd) {
            return;
        }
    }
    if (mOwner->_Unknown11() && !mExpanded) {
        return;
    }

    unsigned long width = 0;
    for (auto& item : mItems) {
        const unsigned long length = std::strlen(item.mText.c_str());
        if (width < length) {
            width = length;
        }
    }
    width += 2;
    if (lines.mLine >= lines.mFirst) {
        TimerItemView::DrawHeader(mOwner, stream, prefix.c_str(), width);
    }
    if (++lines.mLine == lines.mEnd) {
        return;
    }
    for (auto& item : mItems) {
        if (lines.mLine >= lines.mFirst) {
            item.Draw(stream, prefix.c_str(), width, selected && &item == selectedItem);
        }
        if (++lines.mLine == lines.mEnd) {
            return;
        }
    }
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
