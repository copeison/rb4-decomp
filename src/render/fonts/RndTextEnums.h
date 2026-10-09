#pragma once

// The text layout enums (render/RndTextEnums.o). The type names are the
// map's; the enumerator names are not in the reference map and follow the
// display names the text options register (RndTextUtl's tables at 0x67D6C0
// and 0x67D8D0 and the RndTextCom properties at 0x66B430).

// How text that is too wide for its box is handled.
enum RndTextFitMode : int {
    kTextFitModeNone = 0,
    kTextFitModeWordWrap = 1,
    // Cuts the text at the width and ends it with three periods.
    kTextFitModeTruncate = 2,
    // Steps through the style sizes until the text fits the width.
    kTextFitModeShrinkToFit = 3,
    // Wraps the text, then shrinks it until it fits the height.
    kTextFitModeWrapAndShrink = 4,
};

// Vertical alignment of the text against its origin, and of a style within
// its line.
enum RndTextAlignment : int {
    kTextAlignTop = 0,
    kTextAlignMiddle = 1,
    kTextAlignBottom = 2,
};

// Horizontal justification of each line against the origin.
enum RndTextJustification : int {
    kTextJustifyLeft = 0,
    kTextJustifyCenter = 1,
    kTextJustifyRight = 2,
};

// The sizes a style has a font for, largest first. Shrink-to-fit steps
// through them in order.
enum RndFontStyleSize : int {
    kFontStyleSizeRegular = 0,
    kFontStyleSizeSmall = 1,
    kFontStyleSizeVerySmall = 2,
    kFontStyleSizeTiny = 3,
    kFontStyleSizeMiniscule = 4,
};

// The "capitalization_mode" text option. Name not in the reference map.
enum RndTextCapitalization : int {
    kTextCapitalizationMixed = 0,
    kTextCapitalizationLower = 1,
    kTextCapitalizationUpper = 2,
};
