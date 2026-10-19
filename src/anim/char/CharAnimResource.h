#pragma once

#include "anim/core/AnimResource.h"
#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

// Declared for the state graph nodes; not reconstructed.
// A character animation (the map registers it in anim/AnimInit.o): an
// entity resource with the animation interface at offset 200 (vtable
// 0x18F5468). Only the class id and the registration are declared.
class CharAnimResource : public EntityResource, public AnimResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as StateNodeAnimate::_OnResourcesLoaded (0x70EAB0). The local static
    // is at 0x19F7C88.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharAnimResource");
        }
        return id;
    }

    // Registers the class as an entity resource. Emitted in
    // anim/AnimInit.o at 0x2ADD10, which Anim::Init calls.
    static void Init();
};
