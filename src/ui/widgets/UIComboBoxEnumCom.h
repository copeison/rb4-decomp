#pragma once

#include <cstddef>

#include "entity/props/PropArray.h"
#include "ui/widgets/UIComboBoxBaseCom.h"
#include "utl/text/Symbol.h"

// A combo box of enumeration values, each shown with its display text
// ("Combo Box of enums"; ui/UIComboBoxEnumCom.o, 0x898F30-0x89A3FF with the
// static initializer at 0x89A330). Its class symbol is "UIComboBoxEnum".
// The vtable at 0x195C748 has 45 slots. The object is 144 bytes.
class UIComboBoxEnumCom : public UIComboBoxBaseCom<int> {
public:
    // Inlined into the factory and the registration.
    UIComboBoxEnumCom() : mDisplayList() {}
    // The imprint's copy. Inlined into _Imprint.
    UIComboBoxEnumCom(const UIComboBoxEnumCom& other)
        : UIComboBoxBaseCom<int>(other), mDisplayList() {
        mDisplayList._Copy(other.mDisplayList);
    }
    ~UIComboBoxEnumCom() override;  // slots 0-1: 0x899D20, 0x899F60

    Symbol GetId() const override;         // slot 4: 0x89A020
    Symbol GetClassName() const override;  // slot 5: 0x89A030
    int CurrentRev() const override;       // slot 7: 0x89A040
    bool IsA(Symbol type) const override;  // slot 8: 0x89A060
    Component* AsComponent() override;     // slot 9: 0x89A090
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x89A0A0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x89A280
    ComMetaData& _GetMetaData() override;       // slot 23: 0x89A290
    // Slots 41-42: also add and remove the entry's display text. Not
    // reconstructed.
    unsigned int _AddEntryHelper(DataArray* msg) override;          // 0x899DE0
    unsigned int _RemoveEntryHelper(unsigned long index) override;  // 0x899EE0
    // Slot 43: shows the entry's display text, or "<display text not set
    // for enum %d>". Not reconstructed: RndTextCom is not modelled.
    void _UpdateCurrent() override;  // 0x898F30
    // Slot 44: the node's integer.
    int _DataNodeToValue(const DataNode& node) override;  // 0x899070

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x893A10
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x895340) is InitComponentClass's.
    static Component* _Create();  // 0x895250
    // Registers the combo box's properties, then "display_list" and
    // "current_display_value", and describes the class.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x899080

    static Symbol sId;                  // 0x1AC8010, "UIComboBoxEnum"
    // Also "UIComboBoxEnum". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC8018
    static PropRegistry sPropRegistry;  // 0x1AC8020
    static ComMetaData sMetaData;       // 0x1AC80C0

    // "display_list": the display text of each entry. Name not in the
    // reference map.
    PropArray<Symbol> mDisplayList;
};

static_assert(offsetof(UIComboBoxEnumCom, mDisplayList) == 104);
static_assert(sizeof(UIComboBoxEnumCom) == 144);
