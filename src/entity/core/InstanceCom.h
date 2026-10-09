#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "utl/text/Symbol.h"

// The component that instances a child entity resource
// (entity/InstanceCom.o; its registration is at 0x1D21D0). Only the class
// symbols and the members the entity resources use are declared; the class
// is not reconstructed. Field names follow the registry's properties where
// it names them; the others are not in the reference map.
class InstanceCom : public Component {
public:
    static Symbol sId;  // 0x19E4A80
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x19E4A88

    // The members from 23, starting with "num_instances" and the statistics
    // after it at 24; not decoded.
    unsigned char mInstanceStats[93];
    // "instance_polling_mode": always, only when showing, or only when on
    // screen.
    int mInstancePollingMode;
    bool mCopyOnInstance;
    // "drives_parent". Entity::Enter (0xF4D10) and
    // TransEntityResource::_LoadEntity read it on the root's component; the
    // latter keeps the root's transform when it is set.
    bool mDrivesParent;
    bool mStatic;
    // The members from "show_entity" at 128 to the icon data; not decoded.
    unsigned char mDisplayMembers[221];
    // "icon_data"; TransEntityResource::_LoadRoot copies it into the root
    // data of revisions before 12.
    PropArray<unsigned char> mIconData;
    // The members after the icon data; not decoded.
    unsigned char mIconMembers[136];
    // Asks a poll-suppressed TransEntityResource to poll its entity once
    // (0x1BB660), which clears it. Its writer is not identified, so the
    // name is weakly supported.
    bool mPollRequested;
};

static_assert(offsetof(InstanceCom, mInstancePollingMode) == 116);
static_assert(offsetof(InstanceCom, mDrivesParent) == 121);
static_assert(offsetof(InstanceCom, mIconData) == 344);
static_assert(offsetof(InstanceCom, mPollRequested) == 520);
