#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's display of splines
// (render/RndSplineOptionsCom.o, 0x659180-0x65A5BB). Its class id is
// "SplineOptions". The vtable at 0x19304E0 has 41 slots. The object is 40
// bytes.
class RndSplineOptionsCom : public RndOptionsCom {
public:
    RndSplineOptionsCom();  // 0x6591A0
    ~RndSplineOptionsCom() override;  // slots 0-1: 0x6591E0, 0x6591F0

    Symbol GetId() const override;         // slot 4: 0x659D00
    Symbol GetClassName() const override;  // slot 5: 0x659D10
    int CurrentRev() const override;       // slot 7: 0x659D20
    bool IsA(Symbol type) const override;  // slot 8: 0x659D40
    Component* AsComponent() override;     // slot 9: 0x659D70
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x659D80
    // Slot 20: stops being theRndSplineOpts.
    void _PreDestroy(DestroyType type) override;  // 0x659180
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x659ED0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x659EE0
    // Slot 29: becomes theRndSplineOpts.
    bool _OnResourcesLoaded() override;  // 0x659190

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x659210

    static Symbol sId;                  // 0x1AAC548, "SplineOptions"
    // Also "SplineOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1AAC550
    static PropRegistry sPropRegistry;  // 0x1AAC560
    static ComMetaData sMetaData;       // 0x1AAC600

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mShowSplines;  // "show_splines"
    bool mShowSelectedSplineControlPoints;  // "show_selected_spline_control_points"
    bool mShowSelectedSplineControlPointsIndices;  // "show_selected_spline_control_points_indices"
    int mShowSplineHulls;  // "show_spline_hulls"
    bool mShowSelectedSplineContourPoints;  // "show_selected_spline_contour_points"
    bool mShowSelectedSplineContourPointsIndices;  // "show_selected_spline_contour_points_indices"
    bool mDepthTestSplines;  // "depth_test_splines"
};

static_assert(offsetof(RndSplineOptionsCom, mShowSplines) == 0x17);
static_assert(offsetof(RndSplineOptionsCom, mShowSelectedSplineControlPoints) == 0x18);
static_assert(offsetof(RndSplineOptionsCom, mShowSelectedSplineControlPointsIndices) == 0x19);
static_assert(offsetof(RndSplineOptionsCom, mShowSplineHulls) == 0x1C);
static_assert(offsetof(RndSplineOptionsCom, mShowSelectedSplineContourPoints) == 0x20);
static_assert(offsetof(RndSplineOptionsCom, mShowSelectedSplineContourPointsIndices) == 0x21);
static_assert(offsetof(RndSplineOptionsCom, mDepthTestSplines) == 0x22);
static_assert(sizeof(RndSplineOptionsCom) == 0x28);

// The live component, or null while none has loaded its resources.
extern RndSplineOptionsCom* theRndSplineOpts;  // 0x1AAC7A8
