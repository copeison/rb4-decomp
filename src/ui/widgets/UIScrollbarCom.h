#pragma once

#include "entity/core/Component.h"

// A scrollbar widget whose thumb a prop animation positions. (ui/UIScrollbarCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC9BA8.
class UIScrollbarCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x8932B0. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
