#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "math/color/Color.h"
#include "render/options/RndOptionsCom.h"
#include "utl/text/Symbol.h"

// The options for the editor's display of text and the defaults of new text
// (render/RndTextOptionsCom.o, 0x67C560-0x67D42B). Its class id is
// "TextOptions". The vtable at 0x1932A08 has 41 slots. The object is 64
// bytes.
class RndTextOptionsCom : public RndOptionsCom {
public:
    RndTextOptionsCom();  // 0x67C580
    ~RndTextOptionsCom() override;  // slots 0-1: 0x67C610, 0x67C620

    Symbol GetId() const override;         // slot 4: 0x67D170
    Symbol GetClassName() const override;  // slot 5: 0x67D180
    int CurrentRev() const override;       // slot 7: 0x67D190
    bool IsA(Symbol type) const override;  // slot 8: 0x67D1B0
    Component* AsComponent() override;     // slot 9: 0x67D1E0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x67D1F0
    // Slot 20: stops being theRndTextOpts.
    void _PreDestroy(DestroyType type) override;  // 0x67C560
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x67D340
    ComMetaData& _GetMetaData() override;       // slot 23: 0x67D350
    // Slot 29: becomes theRndTextOpts.
    bool _OnResourcesLoaded() override;  // 0x67C570

    // Describes the class and registers its properties after
    // RndOptionsCom::_InitAsSuperclass. Not reconstructed: the property
    // metadata it fills is not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x67C640

    static Symbol sId;                  // 0x1AAE018, "TextOptions"
    // Also "TextOptions". Name not in the reference map.
    static Symbol sClassName;           // 0x1AAE020
    static PropRegistry sPropRegistry;  // 0x1AAE030
    static ComMetaData sMetaData;       // 0x1AAE0D0

    // Field names are not in the reference map; they follow the
    // properties the registry binds to their offsets.
    bool mShowSelectedTextBounds;  // "show_selected_text_bounds"
    bool mShowSelectedTextGlyphBounds;  // "show_selected_text_glyph_bounds"
    Hmx::Color mTextBoundsColor;  // "text_bounds_color"
    int mDisplayMode;  // "FontFamily/display_mode"
    int mAlignment;  // "FontFamily/alignment"
    int mJustification;  // "FontFamily/justification"
    int mStyleSize;  // "FontFamily/style_size"
};

static_assert(offsetof(RndTextOptionsCom, mShowSelectedTextBounds) == 0x17);
static_assert(offsetof(RndTextOptionsCom, mShowSelectedTextGlyphBounds) == 0x18);
static_assert(offsetof(RndTextOptionsCom, mTextBoundsColor) == 0x1C);
static_assert(offsetof(RndTextOptionsCom, mDisplayMode) == 0x2C);
static_assert(offsetof(RndTextOptionsCom, mAlignment) == 0x30);
static_assert(offsetof(RndTextOptionsCom, mJustification) == 0x34);
static_assert(offsetof(RndTextOptionsCom, mStyleSize) == 0x38);
static_assert(sizeof(RndTextOptionsCom) == 0x40);

// The live component, or null while none has loaded its resources.
extern RndTextOptionsCom* theRndTextOpts;  // 0x1AAE278
