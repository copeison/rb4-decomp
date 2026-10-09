#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/time/Timer.h"

// The clocks' time units. The enumerator names are not in the reference map;
// GetTimeUnitsName's tables spell them "seconds", "beats", "uiseconds" and
// "tutorialseconds".
enum TimeUnits {
    kTimeUnitsSeconds = 0,
    kTimeUnitsBeats = 1,
    kTimeUnitsUISeconds = 2,
    kTimeUnitsTutorialSeconds = 3,
    kNumTimeUnits = 4,
};

// The unit's name; `pretty` selects the capitalized spelling.
const char* GetTimeUnitsName(TimeUnits units, bool pretty);  // 0x25A140

// One time unit's clock (the map's Timeline, from eastl::vector<Timeline> in
// utl/TimeMgr.o). Field names are not in the reference map.
struct Timeline {
    double mTime;
    // The time one and two updates ago; deltas measure from one of them.
    double mPrevTime;
    double mPrevPrevTime;
    bool mPaused;
};

static_assert(sizeof(Timeline) == 32);

// The game clocks (utl/TimeMgr.o), created at startup (0x25A470) and
// registered as the "timemgr" data object. Only the members the debug
// overlays read are declared.
class TimeMgr {
public:
    // The clocks of every time unit. The map's TimeMgr has these methods
    // itself; this build moves them into a member that objects can override
    // (the lookup at 0x25A9B0 falls back to TimeMgr's own). Name not in the
    // reference map.
    class Clock {
    public:
        Clock();  // 0x25AA70

        void SetPaused(TimeUnits units, bool paused);  // 0x25A990
        // Sets the time; `reset` also makes the delta zero.
        void SetUISeconds(float seconds, bool reset);  // 0x25AAE0
        float UISeconds() const;                       // 0x25AB20
        float DeltaUISeconds() const;                  // 0x25AB30
        float DeltaTime(TimeUnits units) const;        // 0x25AB60
        void SetDeltaTime(TimeUnits units, float delta);  // 0x25AB90
        void SetTimeAndDelta(TimeUnits units, float time, float delta);  // 0x25ABD0
        void SetSeconds(float seconds, bool reset);    // 0x25AC10
        float Time(TimeUnits units) const;             // 0x25AC50
        float Seconds() const;                         // 0x25AC70
        float DeltaSeconds() const;                    // 0x25AC80
        float Beat() const;                            // 0x25ACA0
        float DeltaBeat() const;                       // 0x25ACB0
        void SetTutorialSeconds(float seconds, bool reset);  // 0x25ACE0
        float TutorialSeconds() const;                 // 0x25AD20
        float DeltaTutorialSeconds() const;            // 0x25AD30

    private:
        float Delta(const Timeline& timeline) const {  // Inlined; name not in the reference map.
            return static_cast<float>(
                timeline.mTime - (mDeltaMode == 1 ? timeline.mPrevPrevTime : timeline.mPrevTime));
        }
        void SetTime(Timeline& timeline, float time, bool reset);  // Inlined; name not in the reference map.

    public:
        // Field names are not in the reference map.
        eastl::vector<Timeline> mTimelines;  // One for each TimeUnits.
        // When 1, deltas span two updates. Weak evidence for its meaning.
        int mDeltaMode;
    };

    // Field names are not in the reference map.
    unsigned char mOpaque0[200];  // Fields the debug overlays do not read.
    // Real time since startup; the overlay graphs plot against it.
    Hmx::Timer mRealTime;
    unsigned char mOpaque224[16];
    // The console overlay blinks its cursor with it.
    Clock mClock;
};

static_assert(sizeof(TimeMgr::Clock) == 40);
static_assert(offsetof(TimeMgr, mRealTime) == 200);
static_assert(offsetof(TimeMgr, mClock) == 240);
static_assert(sizeof(TimeMgr) == 280);

// The live manager. The map's TheTimeMgr is eight bytes, a pointer.
extern TimeMgr* TheTimeMgr;  // 0x19F2520
