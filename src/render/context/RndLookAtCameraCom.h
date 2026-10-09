#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// Points its object at the scene's current camera
// (render/RndLookAtCameraCom.o, 0x4066D0-0x407F2B). At run time it walks up
// the entities to the owning scene and turns the object each poll. Its class
// id is "LookAtCamera". The vtable at 0x18FF8C8 has 41 slots. The object is
// 32 bytes.
class RndLookAtCameraCom : public Component {
public:
    RndLookAtCameraCom();            // 0x4066D0
    ~RndLookAtCameraCom() override;  // slots 0-1: 0x406F30, 0x407A20

    Symbol GetId() const override;         // slot 4: 0x407A40
    Symbol GetClassName() const override;  // slot 5: 0x407A50
    int CurrentRev() const override;       // slot 7: 0x407A60
    bool IsA(Symbol type) const override;  // slot 8: 0x407A80
    Component* AsComponent() override;     // slot 9: 0x407AB0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x407AC0
    // Slot 18: the object's transform is ordered first.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x406F40
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x407BF0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x407C00
    // Slot 27: the scene's main camera polls first. The map's signature
    // starts with an EntityPtr.
    void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) override;  // 0x4078F0
    // Slot 33: turns the object towards the scene's main camera through its
    // transform's local Euler angles.
    void _Poll() override;  // 0x407000
    // Slot 38 at 0x4078E0: polls as in the game mode.
    void _EditPoll() override;

    // Registers "facing_type" and "facing_axis". Not reconstructed: the
    // property metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x406710
    // The class factory. Emitted at 0x4043E0 with the other render
    // factories.
    static Component* _Create();

    static Symbol sId;                  // 0x1A71FC8, "LookAtCamera"
    // Also "LookAtCamera". Name not in the reference map.
    static Symbol sClassName;           // 0x1A71FD0
    static PropRegistry sPropRegistry;  // 0x1A71FE0
    static ComMetaData sMetaData;       // 0x1A72080

    // Field names are not in the reference map; they follow the properties.
    // "facing_type", an RndBillboardType; kBillboardNone does nothing.
    int mFacingType;
    // "facing_axis", an RndBillboardAxis.
    int mFacingAxis;
};

static_assert(offsetof(RndLookAtCameraCom, mFacingType) == 24);
static_assert(offsetof(RndLookAtCameraCom, mFacingAxis) == 28);
static_assert(sizeof(RndLookAtCameraCom) == 32);
