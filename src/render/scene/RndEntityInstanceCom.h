#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class InstanceCom;
class RndDrawNodeCom;

// The rendering side of an InstanceCom (render/RndEntityInstanceCom.o,
// 0x6CAE80 to 0x6CBF3F): it follows the instanced entity's "show_entity"
// and "instance_polling_mode" (EntityHeaderCom) and enables the entity's
// poll only while it shows, or also only while its root's sphere is on
// screen. Its class id is "RndEntityInstance" ("Manages renderer-specific
// aspects of InstanceCom behavior."). The vtable at 0x19383D0 has
// Component's 41 slots. The object is 48 bytes. Field names are not in the
// reference map.
class RndEntityInstanceCom : public Component {
public:
    // The run-time state, which imprints start afresh. The map names the
    // type.
    struct RuntimeData {
        RuntimeData();  // 0x6CAEB0

        // Set while the entity polls only while it shows ("instance_polling_
        // mode" 1 or 2, here or in an enclosing instance).
        bool mPollOnlyWhenShowing;
        // Set while it also polls only while on screen (mode 2).
        bool mPollOnlyOnScreen;
        // The object's instance component and draw node, found by _Enter.
        InstanceCom* mInstance;
        RndDrawNodeCom* mDrawNode;
    };

    RndEntityInstanceCom();  // 0x6CAE80
    // Copies the component for an imprint; the run-time state starts
    // cleared. Inlined into _Imprint (0x6CBD20).
    RndEntityInstanceCom(const RndEntityInstanceCom& other) : Component(other), mRuntime() {}
    // Slots 0-1: 0x6CBC70 (which jumps to Component's) and 0x6CBC80.
    ~RndEntityInstanceCom() override;
    Symbol GetId() const override;          // slot 4: 0x6CBCA0
    Symbol GetClassName() const override;   // slot 5: 0x6CBCB0
    int CurrentRev() const override;        // slot 7: 0x6CBCC0
    bool IsA(Symbol type) const override;   // slot 8: 0x6CBCE0
    Component* AsComponent() override;      // slot 9: 0x6CBD10
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6CBD20
    // Slot 18 at 0x6CB410: follows the draw node and precedes the instance
    // component.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;
    // Slot 20 at 0x6CB100: when the component alone is destroyed, shrinks
    // the draw node's local sphere to the object's position.
    void _PreDestroy(DestroyType type) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6CBE50
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6CBE60
    // Slot 27 at 0x6CB9F0: an entity that depends on the camera polls
    // after the scene camera's object. The map's signature starts with an
    // EntityPtr.
    void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) override;
    // Slot 31 at 0x6CB1E0: finds the instance component and the draw node
    // and works out the polling mode from the instanced entity's header and
    // the enclosing instance.
    void _Enter() override;
    // Slot 33: hides the draw node while "show_entity" is false and
    // enables the instanced entity's poll while it shows and, when asked,
    // while the root's sphere is inside the scene camera's frustum. Not
    // reconstructed: the PropRef evaluation (0x17F610, 0x18B390) and the
    // camera's frustum planes are not modelled.
    void _Poll() override;  // 0x6CB580

    // Whether the instanced entity, or one instanced inside it, polls only
    // while on screen. The map's _HasCameraDependency(EntityPtr); this
    // build asks the component.
    bool _HasCameraDependency() const;  // 0x6CBB40

    // Describes the class: a rendering class for drawable entity
    // resources that requires InstanceCom. It has no properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6CAEC0

    // "RndEntityInstance", constructed by the static initializer at
    // 0x6CBE70.
    static Symbol sId;  // 0x1AB0D48
    // Also "RndEntityInstance". Name not in the reference map.
    static Symbol sClassName;           // 0x1AB0D50
    static PropRegistry sPropRegistry;  // 0x1AB0D60
    static ComMetaData sMetaData;       // 0x1AB0E00

    RuntimeData mRuntime;
};

static_assert(offsetof(RndEntityInstanceCom, mRuntime) == 24);
static_assert(offsetof(RndEntityInstanceCom::RuntimeData, mInstance) == 8);
static_assert(offsetof(RndEntityInstanceCom::RuntimeData, mDrawNode) == 16);
static_assert(sizeof(RndEntityInstanceCom) == 48);
