#pragma once

#include <cstddef>

#include "ui/widgets/UIComboBoxBaseCom.h"
#include "utl/text/Symbol.h"

// A combo box of symbols, shown as they are ("Combo Box of symbols";
// ui/UIComboBoxSymCom.o, 0x89BAB0-0x89C8AF with the static initializer at
// 0x89C7E0). Its class symbol is "UIComboBoxSymbol". The vtable at
// 0x195CA38 has 45 slots. The object is 104 bytes.
class UIComboBoxSymCom : public UIComboBoxBaseCom<Symbol> {
public:
    // Inlined into the factory and the registration.
    UIComboBoxSymCom() {}
    // The imprint's copy. Inlined into _Imprint.
    UIComboBoxSymCom(const UIComboBoxSymCom& other) : UIComboBoxBaseCom<Symbol>(other) {}
    ~UIComboBoxSymCom() override;  // slots 0-1: 0x89C460, 0x89C4D0

    Symbol GetId() const override;         // slot 4: 0x89C550
    Symbol GetClassName() const override;  // slot 5: 0x89C560
    int CurrentRev() const override;       // slot 7: 0x89C570
    bool IsA(Symbol type) const override;  // slot 8: 0x89C590
    Component* AsComponent() override;     // slot 9: 0x89C5C0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x89C5D0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x89C760
    ComMetaData& _GetMetaData() override;       // slot 23: 0x89C770
    // Slot 43: shows the entry in the text object. Not reconstructed:
    // RndTextCom is not modelled.
    void _UpdateCurrent() override;  // 0x89BAB0
    // Slot 44: the symbol a symbol, resource path or string node names.
    Symbol _DataNodeToValue(const DataNode& node) override;  // 0x89BB60

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x8944B0
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x897C80) is InitComponentClass's.
    static Component* _Create();  // 0x897BD0
    // Registers the combo box's properties and describes the class.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x89BBD0

    static Symbol sId;                  // 0x1AC84E8, "UIComboBoxSymbol"
    // Also "UIComboBoxSymbol". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC84F0
    static PropRegistry sPropRegistry;  // 0x1AC8500
    static ComMetaData sMetaData;       // 0x1AC85A0
};

static_assert(sizeof(UIComboBoxSymCom) == 104);
