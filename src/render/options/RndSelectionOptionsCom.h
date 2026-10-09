#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "math/color/Color.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's display of the selection
// (render/RndSelectionOptionsCom.o, 0x46ACD0-0x46B87B). Its class id is
// "SelectionOptions". The vtable at 0x1904880 has 41 slots. The object is 48
// bytes.
class RndSelectionOptionsCom : public RndOptionsCom {
public:
    RndSelectionOptionsCom();  // 0x46ACF0
    ~RndSelectionOptionsCom() override;  // slots 0-1: 0x46AD80, 0x46AD90

    Symbol GetId() const override;         // slot 4: 0x46B3C0
    Symbol GetClassName() const override;  // slot 5: 0x46B3D0
    int CurrentRev() const override;       // slot 7: 0x46B3E0
    bool IsA(Symbol type) const override;  // slot 8: 0x46B400
    Component* AsComponent() override;     // slot 9: 0x46B430
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x46B440
    // Slot 20: stops being theRndSelectionOpts.
    void _PreDestroy(DestroyType type) override;  // 0x46ACD0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x46B590
    ComMetaData& _GetMetaData() override;       // slot 23: 0x46B5A0
    // Slot 29: becomes theRndSelectionOpts.
    bool _OnResourcesLoaded() override;  // 0x46ACE0

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x46ADB0

    static Symbol sId;                  // 0x1A86C38, "SelectionOptions"
    // Also "SelectionOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1A86C40
    static PropRegistry sPropRegistry;  // 0x1A86C50
    static ComMetaData sMetaData;       // 0x1A86CF0

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mShowWireframe;  // "show_wireframe"
    bool mShowBoundingSpheres;  // "show_bounding_spheres"
    bool mShowDistances;  // "show_distances"
    Hmx::Color mDistanceLinesColor;  // "distance_lines_color"
    float mOccludedDistanceLinesAlpha;  // "occluded_distance_lines_alpha"
};

static_assert(offsetof(RndSelectionOptionsCom, mShowWireframe) == 0x17);
static_assert(offsetof(RndSelectionOptionsCom, mShowBoundingSpheres) == 0x18);
static_assert(offsetof(RndSelectionOptionsCom, mShowDistances) == 0x19);
static_assert(offsetof(RndSelectionOptionsCom, mDistanceLinesColor) == 0x1C);
static_assert(offsetof(RndSelectionOptionsCom, mOccludedDistanceLinesAlpha) == 0x2C);
static_assert(sizeof(RndSelectionOptionsCom) == 0x30);

// The live component, or null while none has loaded its resources.
extern RndSelectionOptionsCom* theRndSelectionOpts;  // 0x1A86E98
