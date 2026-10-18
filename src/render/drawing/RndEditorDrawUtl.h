#pragma once

#include "entity/props/PropMetadata.h"
#include "utl/containers/Vector.h"

// Helpers for the editor's in-scene drawing. Only the startup and shutdown
// entry points that Rnd::Init and Rnd::Terminate call, and the choices of
// the editor drawing options, are declared.
class RndEditorDrawUtl {
public:
    // When an option draws its editor helper: the values of options such as
    // RndSplineOptionsCom's "show_spline_hulls". Name and enumerator names
    // not in the reference map; they follow the value names.
    enum DrawWhen : int {
        kDrawNever = 0,
        kDrawIfSelected = 1,
        kDrawAlways = 2,
    };

    // Loads the control-point texture and builds the arrow mesh.
    static void Init();  // 0x461D10
    static void Terminate();  // 0x461F50
    // The DrawWhen choices as allowed values: "Never", "If Selected" and
    // "Always", without help. Name not in the reference map.
    static eastl::vector<AllowedValue<int>> GetDrawWhenAllowedValues();  // 0x461F90
};
