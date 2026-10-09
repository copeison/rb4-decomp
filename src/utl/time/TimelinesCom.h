#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

// The component that gives an entity clocks of its own (0x25FF00-0x260A00);
// its class id is "Timelines". TimeMgr::GetClock finds it on the root
// object of an entity or of the entities instancing it. Only the members the
// renderer uses are declared; the class is not reconstructed. Name not in
// the reference map; it follows the class id.
class TimelinesCom : public Component {
public:
    static Symbol sId;  // 0x19F2748, "Timelines"
    // Also "Timelines"; the metadata's interface, which lookups compare.
    // Name not in the reference map.
    static Symbol sClassName;  // 0x19F2750

    // Field names are not in the reference map.
    TimeMgr::Clock mClock;
    unsigned char mOpaque64[8];  // Not modelled.
    // Whether the scene drawing this entity runs at a partial frame rate.
    // RndSceneCom sets it from the renderer's settings when it loads and
    // enters; the constructor (0x260030) sets it.
    bool mPartialFramerate;
};

static_assert(offsetof(TimelinesCom, mClock) == 24);
static_assert(offsetof(TimelinesCom, mPartialFramerate) == 72);
