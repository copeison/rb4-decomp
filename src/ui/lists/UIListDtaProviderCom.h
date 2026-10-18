#pragma once

#include "entity/core/Component.h"

// Feeds a UIListCom from a DTA file, for prototyping lists before a coded
// provider exists. (ui/UIListDtaProviderCom.o). Only the registration UI::Init calls is declared; the
// class is not reconstructed. Its class symbol is at 0x1AC9350.
class UIListDtaProviderCom : public Component {
public:
    // Registers the class: its property registry, metadata and factory.
    // Inline in the map; emitted in ui/UI.o at 0x892B20. Not reconstructed:
    // the registration helpers it inlines are not modelled.
    static void Init();
};
