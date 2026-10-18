#pragma once

#include "entity/core/Component.h"

// A generic slider, knob or dial with a number of steps. (ui/UISliderCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC9E10.
class UISliderCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x893660. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
