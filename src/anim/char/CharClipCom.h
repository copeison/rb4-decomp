#pragma once

#include <cstddef>

#include "anim/char/CharClipResource.h"
#include "entity/core/Component.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// Declared for the state graph nodes; not reconstructed.
// The component that defines a character clip on its object (class id
// "CharClip", static initializer at 0x2EFEC0). Newer than the reference
// map. Only what the animation nodes use is declared.
class CharClipCom : public Component {
public:
    // Rebuilds the clip and restarts its preview. StateNodeAnimate's
    // StateNodeTiming::Refresh calls it on the clip its node plays. Name not
    // in the reference map; the evidence is weak.
    void Refresh();  // 0x2EA450

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AFE30, which Anim::Init calls.
    static void Init();

    static Symbol sId;  // 0x19F8AA0, "CharClip"

    // The members before the resource; not decoded.
    unsigned char mMembers[192];
    // The clip resource the component builds. Name not in the reference
    // map.
    ResourcePtr<CharClipResource> mResource;
};

static_assert(offsetof(CharClipCom, mResource) == 216);
