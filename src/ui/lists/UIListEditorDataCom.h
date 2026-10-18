#pragma once

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/editor/RndEditorDrawCom.h"
#include "utl/text/Symbol.h"

// The editor-draw counterpart of UIListCom ("RndEditorDrawCom counterpart
// to UIListCom"); its class symbol is "UIListEditorData". It lies before
// UIListItemCom in the binary (0x8C7510-0x8C7B5F with the static
// initializer at 0x8C7AA0), and the map, which is older, has no object for
// it. It draws nothing of its own: it is shown while its object is not
// selected, and nothing picks it. The vtable at 0x195E190 has 59 slots. The
// object is 48 bytes. Name not in the reference map.
class UIListEditorDataCom : public RndEditorDrawCom {
public:
    // Inlined into the factory and the registration.
    UIListEditorDataCom() {}
    ~UIListEditorDataCom() override;  // slots 0-1: 0x8C7890, 0x8C78A0

    Symbol GetId() const override;         // slot 4: 0x8C78C0
    Symbol GetClassName() const override;  // slot 5: 0x8C78D0
    int CurrentRev() const override;       // slot 7: 0x8C78E0
    bool IsA(Symbol type) const override;  // slot 8: 0x8C7900
    Component* AsComponent() override;     // slot 9: 0x8C7930
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8C7940
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8C7A80
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8C7A90
    // Slot 41: 0x14.
    unsigned int _GetCapabilityFlagsImpl() const override;  // 0x8C7840
    // Slot 43: true.
    bool _IsShownUnselectedImpl() const override;  // 0x8C7850
    // Slots 47-49: nothing intersects or is contained, and everything is
    // excluded, as in the base class.
    bool _IntersectSegmentImpl(
        const RndCameraContext& camera,
        const Segment& segment,
        float& t,
        Vector3* point) const override;  // 0x8C7860
    bool _FrustumContainsImpl(
        const RndCameraContext& camera,
        const Frustum& frustum) const override;  // 0x8C7870
    bool _FrustumExcludesImpl(
        const RndCameraContext& camera,
        const Frustum& frustum) const override;  // 0x8C7880

    // Registers the class (InitComponentClass), emitted in ui/UI.o.
    static void Init();  // 0x894960
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x898EE0) is InitComponentClass's.
    static Component* _Create();  // 0x898EA0
    // Describes the class after RndEditorDrawCom::_Init: allowed where
    // UIListCom is, requiring a UIListCom and created with one.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8C7510

    static Symbol sId;                  // 0x1AC95B0, "UIListEditorData"
    // Also "UIListEditorData". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC95B8
    static PropRegistry sPropRegistry;  // 0x1AC95C0
    static ComMetaData sMetaData;       // 0x1AC9660
};

static_assert(sizeof(UIListEditorDataCom) == 48);
