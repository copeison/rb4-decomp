#pragma once

#include <cstddef>

#include "render/debug/RndOverlay.h"

// An overlay that draws a graph of recent samples, keeping room for 200
// 16-byte samples. Name not in the reference map, which may know it as
// RndOverlayStatsBase. The vtable is at 0x1939778; slot 8 (0x6E2740) is not
// declared. Only the declarations are recovered; the members are only
// sized.
class RndOverlayGraphBase : public RndOverlay {
public:
    RndOverlayGraphBase(const char* name, unsigned int flags);  // 0x6E32E0
    ~RndOverlayGraphBase() override;  // slots 0-1: 0x6E34F0, 0x6E3530

    void Draw(RndContext& context, int y) override;  // slot 2: 0x6E3580

    // The sample vector. Name not in the reference map.
    unsigned char mUnknown64[32];
};

static_assert(offsetof(RndOverlayGraphBase, mUnknown64) == 64);
static_assert(sizeof(RndOverlayGraphBase) == 96);

// The "fps_graph" overlay, keeping room for 300 frame times. Name not in the
// reference map. The vtable is at 0x19395A0; slots 8 to 11 are not
// declared.
class RndFramerateGraphOverlay : public RndOverlayGraphBase {
public:
    RndFramerateGraphOverlay();  // 0x6E22A0
    ~RndFramerateGraphOverlay() override;  // slots 0-1: 0x6E2370, 0x6E23B0

    void _Update() override;  // slot 5: 0x6E2400

    // Name not in the reference map.
    unsigned char mUnknown96[32];
};

static_assert(offsetof(RndFramerateGraphOverlay, mUnknown96) == 96);
static_assert(sizeof(RndFramerateGraphOverlay) == 128);

// A graph of the timers of one timers overlay against a budget. Name not
// in the reference map. The vtable is at 0x1939930; slots 8 to 11 are not
// declared.
class RndTimerGraphOverlay : public RndOverlayGraphBase {
public:
    // Looks up the overlay named `timersName`.
    RndTimerGraphOverlay(
        const char* name,
        unsigned int flags,
        const char* timersName,
        float budget);  // 0x6E6280
    ~RndTimerGraphOverlay() override;  // slots 0-1: 0x6E6880, 0x6E6930

    void _Update() override;  // slot 5: 0x6E6950

    // The timers overlay, the graph scale and the per-timer series. Name
    // not in the reference map.
    unsigned char mUnknown96[184];
};

static_assert(offsetof(RndTimerGraphOverlay, mUnknown96) == 96);
static_assert(sizeof(RndTimerGraphOverlay) == 280);

// The "cpu_timer_graph" overlay, against the "cpu" budget category. Name
// not in the reference map. The vtable is at 0x19394A0.
class RndCpuTimerGraphOverlay : public RndTimerGraphOverlay {
public:
    RndCpuTimerGraphOverlay();  // 0x6E1EC0
    ~RndCpuTimerGraphOverlay() override;  // slots 0-1: 0x6E1F40, 0x6E1F50
};

static_assert(sizeof(RndCpuTimerGraphOverlay) == 280);

// The "gpu_timer_graph" overlay, against the "GPU Total" GPU stat budget.
// Name not in the reference map. The vtable is at 0x1939678.
class RndGpuTimerGraphOverlay : public RndTimerGraphOverlay {
public:
    RndGpuTimerGraphOverlay();  // 0x6E2E60
    ~RndGpuTimerGraphOverlay() override;  // slots 0-1: 0x6E2EF0, 0x6E2F00
};

static_assert(sizeof(RndGpuTimerGraphOverlay) == 280);
