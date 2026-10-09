#pragma once

#include <cstddef>

#include "render/debug/overlays/RndOverlayTextBase.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/Vector.h"

// A text overlay listing timers as a tree, with keyboard navigation. In this
// build it is the base of the CPU and GPU timer overlays, which supply the
// timers through slots 11 to 15. The vtable is at 0x19399A0 and has sixteen
// slots.
class RndTimersOverlay : public RndOverlayTextBase {
public:
    // The tree of timer threads. Its destructor (0x6EAB30) frees each
    // thread's view and the timer views inside it. The map names the class;
    // that this member is it rests on that destructor. Field names are not
    // in the reference map.
    class TimedThreadListView {
    public:
        // Inlined into RndTimersOverlay's constructor.
        explicit TimedThreadListView(RndTimersOverlay* owner)
            : mOwner(owner), mUnknown24{} {}
        ~TimedThreadListView();  // 0x6EAB30

        RndTimersOverlay* mOwner;
        LinkedList::Node mThreads;
        // The selected thread and timer views among them; the 'S' key
        // cycles the int at offset 52 through five sorting methods.
        unsigned char mUnknown24[32];
        eastl::vector<void*> mUnknown56;
    };

    // Adds kFlagKeyboard, kFlagHasHelp and kFlagStripedLines to `flags`.
    // The map's signature is RndTimersOverlay().
    RndTimersOverlay(const char* name, unsigned int flags);  // 0x6E7850
    // Slots 0-1: 0x6E78C0, 0x6E78F0.
    ~RndTimersOverlay() override;

    // Not reconstructed: tree navigation and the 'D' and 'S' keys.
    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E7D40
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E8400
    // Not reconstructed.
    void _Update() override;                                     // slot 5: 0x6E79C0
    // Not reconstructed.
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E8420
    Hmx::Color _GetBackgroundColor() const override;             // slot 10: 0x6E8740

    // Slots 11 to 15 are the source interface. Their names are not in the
    // reference map, and their parameter types are only partly recovered.
    // Slot 11 at 0x6E2250: whether the source has threads.
    virtual bool _Unknown11() {
        return true;
    }
    // Slot 12: appends the source's thread keys to `threads`.
    virtual void _Unknown12(eastl::vector<void*>& threads) = 0;
    // Slot 13: appends the timers of the thread with the key to `timers`.
    virtual void _Unknown13(
        void* thread,
        eastl::vector<void*>& timers,
        unsigned int mode,
        int unknown) = 0;
    // Slot 14 at 0x6E2260: prints extra column headers.
    virtual void _Unknown14(TextStream& stream) {
        static_cast<void>(stream);
    }
    // Slot 15 at 0x6E2270: prints the extra columns of `count` timers.
    virtual void _Unknown15(TextStream& stream, const void* timers, unsigned long count) {
        static_cast<void>(stream);
        static_cast<void>(timers);
        static_cast<void>(count);
    }

    TimedThreadListView mThreadList;  // Name not in the reference map.
    // Field names are not in the reference map.
    // Tints the background blue and halves the budget tint.
    bool mUnknown152;
    // How far over budget the timers are, tinting the background towards
    // red; the map's SetOverBudget(float) sets it.
    float mOverBudget;
};

static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mThreads) == 8);
static_assert(offsetof(RndTimersOverlay::TimedThreadListView, mUnknown56) == 56);
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
    // Not reconstructed: copies the per-thread timer tables of the timer
    // registry at 0x19E7D10.
    void _Unknown12(eastl::vector<void*>& threads) override;  // slot 12: 0x6E2030
    // Not reconstructed.
    void _Unknown13(
        void* thread,
        eastl::vector<void*>& timers,
        unsigned int mode,
        int unknown) override;  // slot 13: 0x6E21E0
};

static_assert(sizeof(RndCpuTimersOverlay) == 160);

// The "gpu_timers" overlay. While shown it keeps the GPU statistics
// enabled. Name not in the reference map. The vtable is at 0x19396E8.
class RndGpuTimersOverlay : public RndTimersOverlay {
public:
    RndGpuTimersOverlay();  // 0x6E2F40
    // Slots 0-1: 0x6E2F70, 0x6E2F80.
    ~RndGpuTimersOverlay() override;

    void PrintHelp(TextStream& stream) override;        // slot 4: 0x6E2FC0
    void _HandleShowingChanged(bool showing) override;  // slot 6: 0x6E2FA0
    bool _Unknown11() override;                          // slot 11: 0x6E2FF0
    // Not reconstructed: appends one key, read from 0x19E8810.
    void _Unknown12(eastl::vector<void*>& threads) override;  // slot 12: 0x6E3000
    // Not reconstructed: forwards to RndGpuStatsMgr at 0x62C150.
    void _Unknown13(
        void* thread,
        eastl::vector<void*>& timers,
        unsigned int mode,
        int unknown) override;  // slot 13: 0x6E30C0
    // Not reconstructed: the "num_verts", "num_prims", "vs_invocs",
    // "ps_invocs" and "cs_invocs" columns.
    void _Unknown14(TextStream& stream) override;  // slot 14: 0x6E30E0
    // Not reconstructed.
    void _Unknown15(TextStream& stream, const void* timers, unsigned long count) override;  // slot 15: 0x6E3180
};

static_assert(sizeof(RndGpuTimersOverlay) == 160);
