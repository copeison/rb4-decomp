#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// A scrollbar widget whose thumb a property animation positions ("Com that
// handles everything in scrollbar widget"; ui/UIScrollbarCom.o,
// 0x8CD200-0x8CDAEF with the static initializer at 0x8CDA30). The vtable
// at 0x195E598 has 41 slots. The object is 40 bytes.
class UIScrollbarCom : public Component {
public:
    UIScrollbarCom();  // 0x8CD200
    // The imprint's copy keeps the animation and starts at the top. Inlined
    // into _Imprint. Not in the reference map.
    UIScrollbarCom(const UIScrollbarCom& other)
        : Component(other), mPositionAnim(other.mPositionAnim), mCurrentPosition(0.0F) {}
    ~UIScrollbarCom() override;  // slots 0-1: 0x8CD240, 0x8CD250

    Symbol GetId() const override;         // slot 4: 0x8CD830
    Symbol GetClassName() const override;  // slot 5: 0x8CD840
    int CurrentRev() const override;       // slot 7: 0x8CD850
    bool IsA(Symbol type) const override;  // slot 8: 0x8CD870
    Component* AsComponent() override;     // slot 9: 0x8CD8A0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x8CD8B0
    // Slot 18: the scrollbar follows the animation driver in the component
    // order, without the base class's TransCom dependency.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x8CD610
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8CD9E0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8CD9F0
    // Slot 29: positions the thumb once the animation is loaded. The map
    // has the positioning in _Enter, which this build does not override.
    bool _OnResourcesLoaded() override;  // 0x8CD6D0

    // Clamps the position to [0, 1], keeps it, and applies the animation's
    // frame at that fraction of its range to the entity. Not reconstructed:
    // the animation's frame range and ApplyFrame are not modelled.
    void _UpdateCurPos(float position);  // 0x8CD6F0

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x8932B0
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x8951E0) is InitComponentClass's.
    static Component* _Create();  // 0x8951B0
    // Describes the class (interface "UIScrollbar") and registers
    // "position_anim" and the accessor property "current_position".
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x8CD270

    static Symbol sId;                  // 0x1AC9BA8, "UIScrollbar"
    // Also "UIScrollbar". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC9BB0
    static PropRegistry sPropRegistry;  // 0x1AC9BC0
    static ComMetaData sMetaData;       // 0x1AC9C60

    // Field names are not in the reference map.
    // "position_anim": the PropAnimResource that positions the thumb.
    ResourcePath mPositionAnim;
    // "current_position": the focused element's position, from 0 to 1.
    float mCurrentPosition;
};

static_assert(offsetof(UIScrollbarCom, mPositionAnim) == 24);
static_assert(offsetof(UIScrollbarCom, mCurrentPosition) == 32);
static_assert(sizeof(UIScrollbarCom) == 40);
