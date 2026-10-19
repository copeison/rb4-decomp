#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/props/PropRef.h"

class PropDriver;

// The base of the property-animation key components (the map's
// PropKeysBaseCom, anim/PropKeysCom.o; interface "PropKeys"): the keys of
// one property, "driven_prop", of an object of the entity the animation
// plays on. PropKeysTypeCom<T> holds the TKeyTrack<T> at 264. Only what
// PropAnimDriver uses is declared; the class is not reconstructed.
class PropKeysBaseCom : public Component {
public:
    // Slot 41: a new driver for the property, of the class's type. The map
    // has it as PropKeysFloatCom::CreatePropDriver() const and so on.
    virtual PropDriver* CreatePropDriver() const = 0;

    // "driven_prop": the property the keys drive. Name not in the
    // reference map.
    PropRef mProp;
};

static_assert(offsetof(PropKeysBaseCom, mProp) == 24);
