#pragma once

#include "entity/core/Component.h"

// A selectable object the UI input component focuses and sends input to
// ("Com that receives all input from the UI Manager"). (ui/UIInputReceiverCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC8A50.
class UIInputReceiverCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x891F80. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
