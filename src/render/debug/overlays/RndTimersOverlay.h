#pragma once

#include <_pthread.h>
#include <cstddef>

#include "render/debug/overlays/RndOverlayTextBase.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"

class PerfTimerBase;

// A text overlay listing timers as a tree, with keyboard navigation. In this
// build it is the base of the CPU and GPU timer overlays, which supply the
// timers through slots 11 to 15. The vtable is at 0x19399A0 and has sixteen
// slots.
class RndTimersOverlay : public RndOverlayTextBase {
public:
    // One line of a thread's list: a timer, or the header of the isolated
    // timers or of the timers with ambiguous parents. Field names are not
    // in the reference map.
    class TimerItemView {
    public:
        // The kinds of line. Names not in the reference map.
        enum Type : int {
            kTypeTimer = 0,
            kTypeIsolatedHeader = 1,
            kTypeAmbiguousHeader = 2,
        };

        // Reaches the link of an item in its thread's list.
        class ListNode {
        public:
            static LinkedList::Node& ToNode(TimerItemView& item) {
                return item.mListNode;
            }
            static TimerItemView* FromNode(LinkedList::Node* node) {
                return reinterpret_cast<TimerItemView*>(
                    reinterpret_cast<char*>(node) - offsetof(TimerItemView, mListNode));
            }
        };

        // The map's constructor is TimerItemView(PerfTimer const*). Inlined
        // into ThreadTimersListView::_GatherTimers.
        TimerItemView(RndTimersOverlay* owner, PerfTimerBase* timer, Type type)
            : mOwner(owner), mTimer(timer), mOverBudget(0.0F), mType(type) {
            _UpdateText();
        }

        // A new header line of the type. Name not in the reference map; the
        // out-of-line copy is unreferenced.
        static TimerItemView* _NewHeader(RndTimersOverlay* owner, Type type);  // 0x6E8BF0

        // Prints the column headers after `prefix`, the name column padded
        // to `width`, then the overlay's extra columns.
        static void DrawHeader(
            RndTimersOverlay* overlay,
            TextStream& stream,
            const char* prefix,
            unsigned long width);  // 0x6E9120
        // Prints one column header, padded with underscores to `width`, once
        // per half of a split frame when `split` is set. The overlay is
        // unused. Name not in the reference map.
        static void _PrintHeader(
            RndTimersOverlay* overlay,
            TextStream& stream,
            const char* name,
            unsigned long width,
            bool split);  // 0x6E8C80
        // Prints the text padded with spaces to `width`. The overlay is
        // unused. Name not in the reference map; the out-of-line copy is
        // unreferenced.
        static void _PrintPadded(
            RndTimersOverlay* overlay,
            TextStream& stream,
            const char* text,
            unsigned long width);  // 0x6E8EC0
        // Prints the value with comma separators, padded with spaces to
        // `width`. The map's signature is _PrintStat(TextStream&, unsigned
        // long), a member; in this build the GPU timers overlay passes
        // itself, which is unused, and the value is an int.
        static void _PrintStat(
            RndTimersOverlay* overlay,
            TextStream& stream,
            int value,
            unsigned long width);  // 0x6E8FD0
        // Prints the milliseconds padded to 12 characters. Inlined.
        void _PrintTimer(TextStream& stream, float ms);

        // Rebuilds mText: a timer's indent, fold marker and name, or the
        // header's title. Name not in the reference map.
        void _UpdateText();  // 0x6E88E0
        // Prints the line after `prefix`, marking it when `selected`: the
        // timer's text padded to `width` and its timings, with the budget
        // tinting the background. The map's signature is
        // Draw(RndTimersOverlay*, TextStream&, char const*, unsigned long).
        void Draw(
            TextStream& stream,
            const char* prefix,
            unsigned long width,
            bool selected);  // 0x6E9230

        RndTimersOverlay* mOwner;
        PerfTimerBase* mTimer;  // Null for headers.
        // How far over budget the timer is, decaying when it is not.
        float mOverBudget;
        StackString<128> mText;
        Type mType;
        LinkedList::Node mListNode;
    };

    // The lines Draw prints: the first visible line, the line after the
    // last, and the next line. Name not in the reference map; the field
    // names are not either.
    struct LineRange {
        unsigned long mFirst;
        unsigned long mEnd;
        unsigned long mLine;
    };

    // The timers of one thread. Field names are not in the reference map.
    class ThreadTimersListView {
    public:
        // Reaches the link of a thread view in the thread list.
        class ListNode {
        public:
            static LinkedList::Node& ToNode(ThreadTimersListView& view) {
                return view.mListNode;
            }
            static ThreadTimersListView* FromNode(LinkedList::Node* node) {
                return reinterpret_cast<ThreadTimersListView*>(
                    reinterpret_cast<char*>(node) -
                    offsetof(ThreadTimersListView, mListNode));
            }
        };

        // The order of the thread list: the main thread first. Name not in
        // the reference map.
        class ThreadCmp {
        public:
            bool operator()(
                const ThreadTimersListView& a,
                const ThreadTimersListView& b) const {
                return IsBefore(a, b);
            }
        };

        using ItemList = LinkedList::List<TimerItemView, TimerItemView::ListNode>;

        // The map's constructor is ThreadTimersListView(eastl::pair<pthread*,
        // PerfTimer**> const*, bool); this build's is inlined into
        // TimedThreadListView::Update.
        ThreadTimersListView(RndTimersOverlay* owner, ScePthread thread)
            : mOwner(owner), mThread(thread), mExpanded(false) {}
        // The map's destructor is out of line (0x8B0540 in its build).
        ~ThreadTimersListView() {
            _ClearTimers();
        }

        // Deletes the items. Inlined.
        void _ClearTimers() {
            while (!mItems.empty()) {
                delete &mItems.front();
            }
        }
        // Rebuilds the items from the owner's timers for the thread in the
        // display and sort modes, keeping the views of known timers and
        // headers. A timer is listed when it is selected, isolated, or worse
        // than gTimerThresholdMs under expanded parents; isolated timers and
        // timers with mAmbiguousParent set come after a header.
        // `keepSelection` tells whether `selected` is still listed. The map's
        // signature is _GatherTimers().
        void _GatherTimers(
            TimerItemView* selected,
            unsigned int displayMode,
            int sortMode,
            bool* keepSelection);  // 0x6E9D80
        // Gathers the timers, or drops the items of a folded thread when
        // threads fold. Name not in the reference map; the out-of-line copy
        // is unreferenced.
        void _UpdateTimers(
            TimerItemView* selected,
            unsigned int displayMode,
            int sortMode,
            bool* keepSelection);  // 0x6E9CE0
        // The item of the timer's parent, or null for a header or a timer
        // without a listed parent. Name not in the reference map; the
        // out-of-line copy is unreferenced.
        TimerItemView* _FindParentItem(const TimerItemView& item);  // 0x6E9C90
        // Appends the timers of the timer items to `timers`. Name not in
        // the reference map.
        void AppendTimers(eastl::vector<PerfTimerBase*>& timers);  // 0x6EA8B0
        // The first item, or null. Name not in the reference map.
        TimerItemView* FirstItem() {
            return mItems.empty() ? nullptr : &mItems.front();
        }

        // Whether `a` is listed before `b`: the main thread first, then
        // the poll workers, then the named threads by name, then the others
        // by handle. Name not in the reference map.
        static bool IsBefore(
            const ThreadTimersListView& a,
            const ThreadTimersListView& b);  // 0x6E9A90

        // Prints the thread's line in the lines of `lines`, then, unless
        // the thread is folded, the column headers and the items. The
        // thread line is marked when `selected` and no item is. The map's
        // signature is Draw(RndTimersOverlay*, TextStream&, bool, unsigned
        // long).
        void Draw(
            TextStream& stream,
            bool selected,
            TimerItemView* selectedItem,
            LineRange& lines);  // 0x6EA440

        RndTimersOverlay* mOwner;
        ScePthread mThread;
        ItemList mItems;
        bool mExpanded;
        eastl::vector<PerfTimerBase*> mTimers;
        LinkedList::Node mListNode;
    };

    // The tree of timer threads: the threads, the selection, and the
    // display, sort and scroll state. Field names are not in the reference
    // map.
    class TimedThreadListView {
    public:
        using ThreadList =
            LinkedList::List<ThreadTimersListView, ThreadTimersListView::ListNode>;

        // Inlined into RndTimersOverlay's constructor.
        explicit TimedThreadListView(RndTimersOverlay* owner)
            : mOwner(owner),
              mSelectedThread(nullptr),
              mSelectedItem(nullptr),
              mScroll(0),
              mDisplayMode(0),
              mSortMode(0) {}
        ~TimedThreadListView();  // 0x6EAB30

        // Adds views for new threads, regathers every thread's timers and
        // repairs the selection. Name not in the reference map.
        void Update();  // 0x6E79D0
        // Scrolls the selection into the 30 visible lines. Name not in the
        // reference map.
        void _UpdateScroll();  // 0x6EAC60
        // Prints the display and sort modes and the threshold, then the
        // visible lines of the tree. The map's signature is
        // Draw(RndTimersOverlay*, TextStream&).
        void Draw(TextStream& stream);  // 0x6E8560

        // The arrow keys. Inlined into HandleKeyboardMsg.
        void SelectNextItem();
        void SelectPrevItem();
        void ExpandItem();
        void CollapseItem();

        // The lines the thread view takes. Name not in the reference map.
        unsigned long _NumLines(ThreadTimersListView& view) const;

        RndTimersOverlay* mOwner;
        ThreadList mThreads;
        ThreadTimersListView* mSelectedThread;
        // Null selects the thread itself.
        TimerItemView* mSelectedItem;
        unsigned long mScroll;  // The first visible line.
        int mDisplayMode;       // The 'D' key cycles three.
        int mSortMode;          // The 'S' key cycles five.
        eastl::vector<ScePthread> mThreadKeys;  // Scratch for Update.
    };

    // Adds kFlagKeyboard, kFlagHasHelp and kFlagStripedLines to `flags`.
    // The map's signature is RndTimersOverlay().
    RndTimersOverlay(const char* name, unsigned int flags);  // 0x6E7850
    // Slots 0-1: 0x6E78C0, 0x6E78F0.
    ~RndTimersOverlay() override;

    // The arrow keys move through and fold the tree; 'D' and 'S' cycle the
    // display and sort modes and 'I' isolates the selected timer.
    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E7D40
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E8400
    void _Update() override;                                     // slot 5: 0x6E79C0
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E8420
    Hmx::Color _GetBackgroundColor() const override;             // slot 10: 0x6E8740

    // Slots 11 to 15 are the source interface. Their names are not in the
    // reference map.
    // Slot 11 at 0x6E2250: whether the tree shows thread lines that fold.
    virtual bool _ShowsThreads() {
        return true;
    }
    // Slot 12: appends the source's threads to `threads`.
    virtual void _GatherThreads(eastl::vector<ScePthread>& threads) = 0;
    // Slot 13: replaces `timers` with the thread's timers in the display
    // and sort modes.
    virtual void _GatherThreadTimers(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers,
        unsigned int displayMode,
        int sortMode) = 0;
    // Slot 14 at 0x6E2260: prints extra column headers.
    virtual void _PrintExtraHeaders(TextStream& stream) {
        static_cast<void>(stream);
    }
    // Slot 15 at 0x6E2270: prints the extra columns of the timer's first
    // `numFrames` history frames.
    virtual void _PrintExtraStats(
        TextStream& stream,
        const PerfTimerBase& timer,
        unsigned long numFrames) {
        static_cast<void>(stream);
        static_cast<void>(timer);
        static_cast<void>(numFrames);
    }

    // The background tints for the line being printed. Out of line in the
    // map's build; inlined in this one.
    void SetSelectedItem(bool selected) {
        mIsSelectedItem = selected;
    }
    void SetOverBudget(float overBudget) {
        mOverBudget = overBudget;
    }

    // Replaces `threads` with the source's threads. Name not in the
    // reference map.
    void GetThreads(eastl::vector<ScePthread>& threads);  // 0x6E7930
    // Replaces `timers` with the listed timers of the thread. Name not in
    // the reference map.
    void GetThreadTimers(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers);  // 0x6E7940

    // Shows the timings of the two halves of a split frame side by side.
    static bool gSplitFrameTiming;  // 0x1AB1F04

    TimedThreadListView mThreadList;  // Name not in the reference map.
    // Field names are not in the reference map.
    // Tints the background blue and halves the budget tint; set while the
    // selected line prints.
    bool mIsSelectedItem;
    // How far over budget the timers are, tinting the background towards
    // red; the map's SetOverBudget(float) sets it.
    float mOverBudget;
};

static_assert(offsetof(RndTimersOverlay::TimerItemView, mTimer) == 8);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mOverBudget) == 16);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mText) == 24);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mType) == 176);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mListNode) == 184);
static_assert(sizeof(RndTimersOverlay::TimerItemView) == 200);
static_assert(sizeof(RndTimersOverlay::LineRange) == 24);
static_assert(offsetof(RndTimersOverlay::ThreadTimersListView, mThread) == 8);
static_assert(offsetof(RndTimersOverlay::ThreadTimersListView, mItems) == 16);
static_assert(offsetof(RndTimersOverlay::ThreadTimersListView, mExpanded) == 32);
static_assert(offsetof(RndTimersOverlay::ThreadTimersListView, mTimers) == 40);
static_assert(offsetof(RndTimersOverlay::ThreadTimersListView, mListNode) == 72);
static_assert(sizeof(RndTimersOverlay::ThreadTimersListView) == 88);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mThreads) == 8);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mSelectedThread) == 24);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mSelectedItem) == 32);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mScroll) == 40);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mDisplayMode) == 48);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mSortMode) == 52);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mThreadKeys) == 56);
static_assert(sizeof(RndTimersOverlay::TimedThreadListView) == 88);
static_assert(offsetof(RndTimersOverlay, mThreadList) == 64);
static_assert(offsetof(RndTimersOverlay, mIsSelectedItem) == 152);
static_assert(offsetof(RndTimersOverlay, mOverBudget) == 156);
static_assert(sizeof(RndTimersOverlay) == 160);

// The "cpu_timers" overlay. Name not in the reference map; its object
// links between those of the CPU timer graph and the framerate graph. The
// vtable is at 0x1939510.
class RndCpuTimersOverlay : public RndTimersOverlay {
public:
    RndCpuTimersOverlay();  // 0x6E1F90
    // Slots 0-1: 0x6E1FC0, 0x6E1FD0.
    ~RndCpuTimersOverlay() override;

    void PrintHelp(TextStream& stream) override;  // slot 4: 0x6E1FF0
    void _GatherThreads(eastl::vector<ScePthread>& threads) override;  // slot 12: 0x6E2030
    void _GatherThreadTimers(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers,
        unsigned int displayMode,
        int sortMode) override;  // slot 13: 0x6E21E0
};

static_assert(sizeof(RndCpuTimersOverlay) == 160);

// The "gpu_timers" overlay: the GPU statistics as one list. While shown it
// keeps the GPU statistics enabled. Name not in the reference map. The
// vtable is at 0x19396E8.
class RndGpuTimersOverlay : public RndTimersOverlay {
public:
    RndGpuTimersOverlay();  // 0x6E2F40
    // Slots 0-1: 0x6E2F70, 0x6E2F80.
    ~RndGpuTimersOverlay() override;

    void PrintHelp(TextStream& stream) override;        // slot 4: 0x6E2FC0
    void _HandleShowingChanged(bool showing) override;  // slot 6: 0x6E2FA0
    bool _ShowsThreads() override;                          // slot 11: 0x6E2FF0
    void _GatherThreads(eastl::vector<ScePthread>& threads) override;  // slot 12: 0x6E3000
    void _GatherThreadTimers(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers,
        unsigned int displayMode,
        int sortMode) override;  // slot 13: 0x6E30C0
    // The "num_verts", "num_prims", "vs_invocs", "ps_invocs" and
    // "cs_invocs" columns.
    void _PrintExtraHeaders(TextStream& stream) override;  // slot 14: 0x6E30E0
    void _PrintExtraStats(
        TextStream& stream,
        const PerfTimerBase& timer,
        unsigned long numFrames) override;  // slot 15: 0x6E3180
};

static_assert(sizeof(RndGpuTimersOverlay) == 160);
