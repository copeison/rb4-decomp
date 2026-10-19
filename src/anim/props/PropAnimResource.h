#pragma once

#include <cstddef>

#include "anim/core/AnimResource.h"
#include "entity/core/EntityResource.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Entity;
class PropKeysBaseCom;

// A property animation (anim/PropAnimResource.o): an entity resource
// whose objects hold the PropKeys components that key properties of the
// entity it plays on, with the animation interface at offset 200 (vtables
// 0x18F7190 and 0x18F7308). Its factory (0x2B1DC0) allocates 216 bytes.
// Only what the drivers, the state graph and the UI use is declared; the
// class is not reconstructed.
class PropAnimResource : public EntityResource, public AnimResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as UIScrollbarCom::_Init (0x8CD270) and AnimDriverCom::_FindDriver
    // (0x32A7D0). The local static is at 0x19F7BC8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("PropAnimResource");
        }
        return id;
    }

    // Registers the class as an entity resource. Emitted in
    // anim/AnimInit.o at 0x2A7B00, which Anim::Init calls.
    static void Init();

    // Sets the keyed properties of the entity to their values at the
    // frame. The map's ApplyFrame(float, EntityPtr const&) const.
    void ApplyFrame(float frame, Entity* entity) const;  // 0x2FF4C0

    // Appends the key components of the entity's objects other than the
    // root. What the flag selects is not decoded. Name not in the reference
    // map; not reconstructed.
    void GetPropKeys(eastl::vector<PropKeysBaseCom*>& keys, bool flag) const;  // 0x2FEAE0

    // Not decoded; 0 after construction.
    int mMembers208;
};

static_assert(sizeof(PropAnimResource) == 216);
