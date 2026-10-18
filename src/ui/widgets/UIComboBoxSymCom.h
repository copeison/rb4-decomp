#pragma once

#include "entity/core/Component.h"

// A combo box of symbols, built on the map's UIComboBoxBaseCom<Symbol>. (ui/UIComboBoxSymCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC84E8.
class UIComboBoxSymCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x8944B0. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
