#pragma once

#include <cstddef>

#include "render/debug/overlays/RndOverlayTextBase.h"

class PerfTimer;

// The "framerate" overlay: the output resolution and smoothed framerate,
// optionally followed by the average CPU and GPU frame times. The vtable is
// at 0x1939610.
class RndFramerateOverlay : public RndOverlayTextBase {
public:
    RndFramerateOverlay();  // 0x6E27A0
    // Slots 0-1: 0x6E2830, 0x6E2840.
    ~RndFramerateOverlay() override;

    // 'C' toggles the CPU average and 'G' the GPU average.
    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E2860
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E2940
    void _HandleShowingChanged(bool showing) override;           // slot 6: 0x6E2910
    // Adds the "show_cpu_average" and "show_gpu_average" options.
    void _RegisterOptions(PropRegistry& registry) override;             // slot 7: 0x6E2960
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E2A60
    Hmx::Color _GetBackgroundColor() const override;             // slot 10: 0x6E2CA0

    // Prints the averages of both halves of a split frame when set. At
    // 0x1AB1DB0.
    static bool gSplitFrameTiming;

    // Field names are not in the reference map.
    bool mShowCpuAverage;
    // While shown with this set, the overlay keeps GPU statistics enabled.
    bool mShowGpuAverage;
    PerfTimer* mCpuTimer;  // The "cpu" timer.
};

static_assert(offsetof(RndFramerateOverlay, mShowCpuAverage) == 64);
static_assert(offsetof(RndFramerateOverlay, mShowGpuAverage) == 65);
static_assert(offsetof(RndFramerateOverlay, mCpuTimer) == 72);
static_assert(sizeof(RndFramerateOverlay) == 80);
