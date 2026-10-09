#pragma once

#include <cstddef>

// The base of every entity component (entity/Component.o). Its vtable and
// methods are not reconstructed; only the fields that Component::Component
// (0xE7190) sets and the flag that the renderer's default lighting sets on
// its light and light-probe components are declared. The fields are bytes so
// that the class keeps an alignment of one: the subclasses' layouts place
// their first members right after mEnabled.
class Component {
public:
    // A description for error messages, "%s component in %s" from the
    // class name and the object's MakeErrorName. The map's
    // MakeErrorName(ObjPtr const&) const; this build uses the owning object.
    const char* MakeErrorName() const;  // 0xE8280

    // The vtable pointer; the virtual methods are not reconstructed. Name not
    // in the reference map.
    unsigned char mVtable[8];
    // The GameObject* that owns the component, set by
    // GameObject::CreateComponent's body (0x116AE0). Name not in the
    // reference map.
    unsigned char mObject[8];
    // Flags the constructor sets to 0, 1, 0, 0, 0 and 0; their meaning is not
    // recovered. Name not in the reference map.
    unsigned char mStateFlags[6];
    // Cleared to switch the component off. RndDefaults::_LoadLighting
    // (0x6BEF40) and _SyncEnabledLights (0x6BFA60) write it on RndLightCom
    // and RndLightProbeCom components. Name not in the reference map.
    bool mEnabled;
};

static_assert(offsetof(Component, mObject) == 8);
static_assert(offsetof(Component, mStateFlags) == 16);
static_assert(offsetof(Component, mEnabled) == 22);
