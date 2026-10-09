#pragma once

#include <cstddef>

#include "utl/time/Timer.h"

// The game clocks (utl/TimeMgr.o), created at startup (0x25A470) and
// registered as the "timemgr" data object. Only the members the debug
// overlays read are declared.
class TimeMgr {
public:
    // A clock reached through a pointer to its state. Name not in the
    // reference map.
    class Clock {
    public:
        // The clock's time in seconds.
        float Seconds() const;  // 0x25AB20
        // The current time of the clock's first timebase, in seconds. The
        // state holds 32-byte timebases; Seconds reads the third. At
        // 0x25AC70. Name not in the reference map.
        float DefaultTime() const;

        // Field names are not in the reference map.
        struct State {
            double mDefaultTime;  // Name not in the reference map.
            unsigned char mOpaque8[56];  // The rest of the first timebases.
            double mSeconds;
        };

        State* mState;
    };

    // Field names are not in the reference map.
    unsigned char mOpaque0[200];  // Fields the debug overlays do not read.
    // Real time since startup; the overlay graphs plot against it.
    Hmx::Timer mRealTime;
    unsigned char mOpaque224[16];
    // The console overlay blinks its cursor with it.
    Clock mClock;
};

static_assert(offsetof(TimeMgr::Clock::State, mSeconds) == 64);
static_assert(offsetof(TimeMgr, mRealTime) == 200);
static_assert(offsetof(TimeMgr, mClock) == 240);

// The live manager. The map's TheTimeMgr is eight bytes, a pointer.
extern TimeMgr* TheTimeMgr;  // 0x19F2520
