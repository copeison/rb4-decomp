// The "Timelines" component's object (0x25FF00 to 0x260A00). Only the clock
// functions the scene resource calls are reconstructed.
#include "utl/time/TimelinesCom.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"

// Reconstructed from eboot.elf at 0x260520.
void TimelinesCom::_SyncClock() {
    TimeMgr::Clock* const parent = mParentClock;
    if (&mClock != parent) {
        mClock.mTimelines = parent->mTimelines;
    }
    mClock.mDeltaMode = parent->mDeltaMode;
    if (mPartialFramerate) {
        if (mFramerate == 1) {
            mClock.mDeltaMode = 1;
        }
    } else if (mClock.mDeltaMode == 1) {
        static_cast<void>(MakeErrorName());
        mClock.mDeltaMode = 0;
    }
    _OnClockSynced();
}

// Reconstructed from eboot.elf at 0x260720.
void TimelinesCom::_UpdateParentClock() {
    Entity* const entity = mObject->mEntity;
    if ((entity->mFlags & 0x4000) != 0) {
        GameObject* const parent = entity->mParentObject;
        mParentClock = TheTimeMgr->GetClock(parent != nullptr ? parent->mEntity : nullptr);
    }
}
