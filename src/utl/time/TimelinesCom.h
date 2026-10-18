#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

// The component that gives an entity clocks of its own (0x25FF00-0x260A00);
// its class id is "Timelines". TimeMgr::GetClock finds it on the root
// object of an entity or of the entities instancing it. Its clock follows
// the clock of the entity's parent. Only the members the renderer uses are
// declared; the class is not reconstructed. The vtable at 0x18EFC98 has 42
// slots. Name not in the reference map; it follows the class id.
class TimelinesCom : public Component {
public:
    // Slot 41 at 0x260950: told after _SyncClock copied the parent clock;
    // empty here. Name not in the reference map.
    virtual void _OnClockSynced() {}

    // Looks up the parent entity's clock again when the entity's parent
    // changed (Entity flag 0x4000). Name not in the reference map.
    void _UpdateParentClock();  // 0x260720
    // Copies the parent clock's timelines and delta mode. At a partial
    // frame rate a "framerate" of half makes deltas span two updates;
    // otherwise two-update deltas are reported (the report is compiled
    // out) and reset. Name not in the reference map.
    void _SyncClock();  // 0x260520

    static Symbol sId;  // 0x19F2748, "Timelines"
    // Also "Timelines"; the metadata's interface, which lookups compare.
    // Name not in the reference map.
    static Symbol sClassName;  // 0x19F2750

    // Field names are not in the reference map.
    TimeMgr::Clock mClock;
    // The clock of the entity's parent, or TimeMgr's (TimeMgr::GetClock).
    TimeMgr::Clock* mParentClock;
    // Whether the scene drawing this entity runs at a partial frame rate.
    // RndSceneCom sets it from the renderer's settings when it loads and
    // enters; the constructors (0x25FF50, 0x260030) set it.
    bool mPartialFramerate;
    // Set when the scene resource synced the clock this frame, so the
    // component's poll (0x2606C0) skips its own sync and clears it.
    bool mClockSynced;
    unsigned char mPadding74[6];  // Never read or written.
    // "framerate": 0 full, 1 half.
    int mFramerate;
};

static_assert(offsetof(TimelinesCom, mClock) == 24);
static_assert(offsetof(TimelinesCom, mParentClock) == 64);
static_assert(offsetof(TimelinesCom, mPartialFramerate) == 72);
static_assert(offsetof(TimelinesCom, mClockSynced) == 73);
static_assert(offsetof(TimelinesCom, mFramerate) == 80);
