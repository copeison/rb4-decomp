#pragma once

#include "entity/core/Component.h"

// A scrolling list of instanced entities ("Lists of selectable entities"). (ui/UIListCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC9038.
class UIListCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x892740. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
