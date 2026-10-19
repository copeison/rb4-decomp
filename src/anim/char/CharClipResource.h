#pragma once

#include "anim/core/AnimResource.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// Declared for the state graph nodes; not reconstructed.
// A blend-tree-compatible character clip, as a CharClipCom defines it: a
// resource with the animation interface at offset 48 (vtable 0x18F68D0).
// Newer than the reference map. Only the class id and the registration are
// declared.
class CharClipResource : public Resource, public AnimResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as StateNodeAnimate::GetAnimResource (0x70F0F0). The local static is
    // at 0x19F7D38.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharClipResource");
        }
        return id;
    }

    // Registers the class as a resource. Emitted in anim/AnimInit.o at
    // 0x2AFCC0, which Anim::Init calls.
    static void Init();
};
