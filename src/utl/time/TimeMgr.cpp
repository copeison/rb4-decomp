#include "utl/time/TimeMgr.h"

namespace {

// The unit names, at 0x18EFC00 and 0x18EFC30. Names not in the reference
// map.
const char* const gTimeUnitsNames[] = {
    "seconds", "beats", "uiseconds", "tutorialseconds", "num units"};
const char* const gTimeUnitsPrettyNames[] = {
    "Seconds", "Beats", "UI Seconds", "Tutorial Seconds", "Num Units"};

}  // namespace

TimeMgr* TheTimeMgr;

// Reconstructed from eboot.elf at 0x25A140. The map's signature is
// GetTimeUnitsName(TimeUnits).
const char* GetTimeUnitsName(TimeUnits units, bool pretty) {
    return (pretty ? gTimeUnitsPrettyNames : gTimeUnitsNames)[units];
}

// Reconstructed from eboot.elf at 0x25A990.
void TimeMgr::Clock::SetPaused(TimeUnits units, bool paused) {
    Timeline& timeline = mTimelines[units];
    timeline.mPaused = paused;
    timeline.mPrevTime = timeline.mTime;
    timeline.mPrevPrevTime = timeline.mTime;
}

// Reconstructed from eboot.elf at 0x25AA70.
TimeMgr::Clock::Clock() : mDeltaMode(0) {
    mTimelines.resize(kNumTimeUnits);
}

void TimeMgr::Clock::SetTime(Timeline& timeline, float time, bool reset) {
    if (reset) {
        timeline.mPrevPrevTime = time;
        timeline.mPrevTime = time;
    } else {
        timeline.mPrevPrevTime = timeline.mPrevTime;
        timeline.mPrevTime = timeline.mTime;
    }
    timeline.mTime = time;
}

// Reconstructed from eboot.elf at 0x25AAE0.
void TimeMgr::Clock::SetUISeconds(float seconds, bool reset) {
    SetTime(mTimelines[kTimeUnitsUISeconds], seconds, reset);
}

// Reconstructed from eboot.elf at 0x25AB20.
float TimeMgr::Clock::UISeconds() const {
    return static_cast<float>(mTimelines[kTimeUnitsUISeconds].mTime);
}

// Reconstructed from eboot.elf at 0x25AB30.
float TimeMgr::Clock::DeltaUISeconds() const {
    return Delta(mTimelines[kTimeUnitsUISeconds]);
}

// Reconstructed from eboot.elf at 0x25AB60.
float TimeMgr::Clock::DeltaTime(TimeUnits units) const {
    return Delta(mTimelines[units]);
}

// Reconstructed from eboot.elf at 0x25AB90.
void TimeMgr::Clock::SetDeltaTime(TimeUnits units, float delta) {
    Timeline& timeline = mTimelines[units];
    const double prevTime = timeline.mTime - static_cast<double>(delta);
    const double span = timeline.mPrevPrevTime - timeline.mPrevTime;
    timeline.mPrevTime = prevTime;
    timeline.mPrevPrevTime = span + prevTime;
}

// Reconstructed from eboot.elf at 0x25ABD0.
void TimeMgr::Clock::SetTimeAndDelta(TimeUnits units, float time, float delta) {
    Timeline& timeline = mTimelines[units];
    const double prevTime = static_cast<double>(time - delta);
    const double prevPrevTime = prevTime - timeline.mTime + timeline.mPrevTime;
    timeline.mTime = static_cast<double>(time);
    timeline.mPrevTime = prevTime;
    timeline.mPrevPrevTime = prevPrevTime;
}

// Reconstructed from eboot.elf at 0x25AC10.
void TimeMgr::Clock::SetSeconds(float seconds, bool reset) {
    SetTime(mTimelines[kTimeUnitsSeconds], seconds, reset);
}

// Reconstructed from eboot.elf at 0x25AC50.
float TimeMgr::Clock::Time(TimeUnits units) const {
    return static_cast<float>(mTimelines[units].mTime);
}

// Reconstructed from eboot.elf at 0x25AC70.
float TimeMgr::Clock::Seconds() const {
    return static_cast<float>(mTimelines[kTimeUnitsSeconds].mTime);
}

// Reconstructed from eboot.elf at 0x25AC80.
float TimeMgr::Clock::DeltaSeconds() const {
    return Delta(mTimelines[kTimeUnitsSeconds]);
}

// Reconstructed from eboot.elf at 0x25ACA0.
float TimeMgr::Clock::Beat() const {
    return static_cast<float>(mTimelines[kTimeUnitsBeats].mTime);
}

// Reconstructed from eboot.elf at 0x25ACB0.
float TimeMgr::Clock::DeltaBeat() const {
    return Delta(mTimelines[kTimeUnitsBeats]);
}

// Reconstructed from eboot.elf at 0x25ACE0.
void TimeMgr::Clock::SetTutorialSeconds(float seconds, bool reset) {
    SetTime(mTimelines[kTimeUnitsTutorialSeconds], seconds, reset);
}

// Reconstructed from eboot.elf at 0x25AD20.
float TimeMgr::Clock::TutorialSeconds() const {
    return static_cast<float>(mTimelines[kTimeUnitsTutorialSeconds].mTime);
}

// Reconstructed from eboot.elf at 0x25AD30.
float TimeMgr::Clock::DeltaTutorialSeconds() const {
    return Delta(mTimelines[kTimeUnitsTutorialSeconds]);
}
