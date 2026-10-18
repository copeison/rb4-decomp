#pragma once

#include "entity/props/PropMetadata.h"
#include "render/fonts/RndTextEnums.h"
#include "utl/containers/Vector.h"

// The names and choices of the text layout enums for the editor
// (render/RndTextUtl.o, 0x67D470-0x67D8EF). The map's GetAlignmentName,
// GetAlignmentHelp, GetJustificationName and GetJustificationHelp are
// inlined into the choice lists in this build and are not declared.
class RndTextUtl {
public:
    // The alignments with their names and help, such as "Top" and "Align to
    // top".
    static eastl::vector<AllowedValue<int>> GetAlignmentAllowedValues();  // 0x67D470
    // The justifications, such as "Left" and "Justify to the left".
    static eastl::vector<AllowedValue<int>> GetJustificationAllowedValues();  // 0x67D6C0
    // The size's name, such as "Very Small", or the empty name for any other
    // value.
    static const char* GetFontStyleSizeName(RndFontStyleSize size);  // 0x67D8D0
};
