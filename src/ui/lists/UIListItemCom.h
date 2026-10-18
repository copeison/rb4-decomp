#pragma once

#include "entity/core/Component.h"

// The interface between a UIListCom and one of its items. (ui/UIListItemCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC9810.
class UIListItemCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x892F00. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
