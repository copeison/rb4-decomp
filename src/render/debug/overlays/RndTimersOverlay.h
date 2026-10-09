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
    // One line of a thread's list. The map's constructor is
    // TimerItemView(PerfTimer const*); this build builds the items inline
    // in ThreadTimersListView::_GatherTimers. Field names are not in the
    // reference map.
    class TimerItemView {
    public:
        // The kinds of line. Names not in the reference map.
        enum Type : int {
            kTypeTimer = 0,
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

        // Prints a column header, padded with underscores to `width`, once
        // per half of a split frame when `split` is set. The map's
        // signature has no split argument.
        static void DrawHeader(
            RndTimersOverlay* overlay,
            TextStream& stream,
            const char* name,
            unsigned long width,
            bool split);  // 0x6E8C80
        // Prints the value with comma separators, padded with spaces to
        // `width`. The map's signature is _PrintStat(TextStream&, unsigned
        // long), a member.
        static void _PrintStat(
            RndTimersOverlay* overlay,
            TextStream& stream,
            unsigned int value,
            unsigned long width);  // 0x6E8FD0

        RndTimersOverlay* mOwner;
        PerfTimerBase* mTimer;  // For kTypeTimer.
        int mUnknown16;
        StackString<128> mText;
        Type mType;
        LinkedList::Node mListNode;
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
        // display and sort modes, keeping the views of known timers.
        // `keepSelection` is cleared when `selected` is no longer listed.
        // The map's signature is _GatherTimers().
        void _GatherTimers(
            TimerItemView* selected,
            unsigned int displayMode,
            int sortMode,
            bool* keepSelection);  // 0x6E9D80
        // Appends the timers of the timer items to `timers`. Name not in
        // the reference map.
        void AppendTimers(eastl::vector<PerfTimerBase*>& timers);  // 0x6EA8B0
        // The first item, or null. Name not in the reference map.
        TimerItemView* FirstItem() {
            return mItems.empty() ? nullptr : &mItems.front();
        }

        // Whether `a` is listed before `b`: the main thread first, then
        // the threads by name, then by handle. Name not in the reference
        // map.
        static bool IsBefore(
            const ThreadTimersListView& a,
            const ThreadTimersListView& b);  // 0x6E9A90

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
        // Prints the tree. The map's signature is
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
    virtual bool _Unknown11() {
        return true;
    }
    // Slot 12: appends the source's threads to `threads`.
    virtual void _Unknown12(eastl::vector<ScePthread>& threads) = 0;
    // Slot 13: replaces `timers` with the thread's timers in the display
    // and sort modes.
    virtual void _Unknown13(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers,
        unsigned int displayMode,
        int sortMode) = 0;
    // Slot 14 at 0x6E2260: prints extra column headers.
    virtual void _Unknown14(TextStream& stream) {
        static_cast<void>(stream);
    }
    // Slot 15 at 0x6E2270: prints the extra columns of the timer's first
    // `numFrames` history frames.
    virtual void _Unknown15(
        TextStream& stream,
        const PerfTimerBase& timer,
        unsigned long numFrames) {
        static_cast<void>(stream);
        static_cast<void>(timer);
        static_cast<void>(numFrames);
    }

    // Replaces `threads` with the source's threads. Name not in the
    // reference map.
    void GetThreads(eastl::vector<ScePthread>& threads);  // 0x6E7930
    // Replaces `timers` with the listed timers of the thread. Name not in
    // the reference map.
    void GetThreadTimers(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers);  // 0x6E7940

    TimedThreadListView mThreadList;  // Name not in the reference map.
    // Field names are not in the reference map.
    // Tints the background blue and halves the budget tint.
    bool mUnknown152;
    // How far over budget the timers are, tinting the background towards
    // red; the map's SetOverBudget(float) sets it.
    float mOverBudget;
};

static_assert(offsetof(RndTimersOverlay::TimerItemView, mTimer) == 8);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mText) == 24);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mType) == 176);
static_assert(offsetof(RndTimersOverlay::TimerItemView, mListNode) == 184);
static_assert(sizeof(RndTimersOverlay::TimerItemView) == 200);
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
static_assert(offsetof(RndTimersOverlay, mUnknown152) == 152);
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
    void _Unknown12(eastl::vector<ScePthread>& threads) override;  // slot 12: 0x6E2030
    void _Unknown13(
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
    bool _Unknown11() override;                          // slot 11: 0x6E2FF0
    void _Unknown12(eastl::vector<ScePthread>& threads) override;  // slot 12: 0x6E3000
    void _Unknown13(
        ScePthread thread,
        eastl::vector<PerfTimerBase*>& timers,
        unsigned int displayMode,
        int sortMode) override;  // slot 13: 0x6E30C0
    // The "num_verts", "num_prims", "vs_invocs", "ps_invocs" and
    // "cs_invocs" columns.
    void _Unknown14(TextStream& stream) override;  // slot 14: 0x6E30E0
    void _Unknown15(
        TextStream& stream,
        const PerfTimerBase& timer,
        unsigned long numFrames) override;  // slot 15: 0x6E3180
};

static_assert(sizeof(RndGpuTimersOverlay) == 160);
