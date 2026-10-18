#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that drives an object's animations from its timelines
// (anim/AnimDriverCom.o; its registration is at 0x329900, class id
// "AnimDriver"). Only the class symbols that the scene resource uses are
// declared; the class is not reconstructed.
class AnimDriverCom : public Component {
public:
    static Symbol sId;  // 0x19FB690
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x19FB698

    // "default_timeline": the TimeUnits the state graphs' transitions use
    // when they ask for the driver's (StateGraphInstance::ResolveTimeUnits).
    // Name not in the reference map.
    int mDefaultTimeline;
};

static_assert(offsetof(AnimDriverCom, mDefaultTimeline) == 24);
