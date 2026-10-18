#include "render/fonts/RndTextUtl.h"

namespace {

// The style size names (0x1932BD0). Name not in the reference map.
const char* const kFontStyleSizeNames[kNumFontStyleSizes] = {
    "Regular",
    "Small",
    "Very Small",
    "Tiny",
    "Miniscule",
};

}  // namespace

// Reconstructed from eboot.elf at 0x67D470.
eastl::vector<AllowedValue<int>> RndTextUtl::GetAlignmentAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(3);
    values.push_back(AllowedValue<int>{kTextAlignTop, String("Top"), String("Align to top")});
    values.push_back(AllowedValue<int>{kTextAlignMiddle, String("Middle"), String("Align to middle")});
    values.push_back(AllowedValue<int>{kTextAlignBottom, String("Bottom"), String("Align to bottom")});
    return values;
}

// Reconstructed from eboot.elf at 0x67D6C0.
eastl::vector<AllowedValue<int>> RndTextUtl::GetJustificationAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(3);
    values.push_back(AllowedValue<int>{kTextJustifyLeft, String("Left"), String("Justify to the left")});
    values.push_back(
        AllowedValue<int>{kTextJustifyCenter, String("Center"), String("Justify to the center")});
    values.push_back(AllowedValue<int>{kTextJustifyRight, String("Right"), String("Justify to the right")});
    return values;
}

// Reconstructed from eboot.elf at 0x67D8D0.
const char* RndTextUtl::GetFontStyleSizeName(RndFontStyleSize size) {
    return static_cast<unsigned int>(size) < static_cast<unsigned int>(kNumFontStyleSizes)
        ? kFontStyleSizeNames[size]
        : Symbol().Str();
}
