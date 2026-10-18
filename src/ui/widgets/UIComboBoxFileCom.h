#pragma once

#include <cstddef>

#include "entity/props/PropArray.h"
#include "ui/widgets/UIComboBoxBaseCom.h"
#include "utl/text/Symbol.h"

// A combo box of files, each shown with its display text ("Combo Box of
// files"; ui/UIComboBoxFileCom.o, 0x89A400-0x89BAAF with the static
// initializer at 0x89B9E0). Its class symbol is "UIComboBoxFile". The
// vtable at 0x195C8C0 has 45 slots. The object is 144 bytes.
class UIComboBoxFileCom : public UIComboBoxBaseCom<ResourcePath> {
public:
    // Inlined into the factory and the registration.
    UIComboBoxFileCom() : mDisplayList() {}
    // The imprint's copy. Inlined into _Imprint.
    UIComboBoxFileCom(const UIComboBoxFileCom& other)
        : UIComboBoxBaseCom<ResourcePath>(other), mDisplayList() {
        mDisplayList._Copy(other.mDisplayList);
    }
    ~UIComboBoxFileCom() override;  // slots 0-1: 0x89B3C0, 0x89B600

    Symbol GetId() const override;         // slot 4: 0x89B6C0
    Symbol GetClassName() const override;  // slot 5: 0x89B6D0
    int CurrentRev() const override;       // slot 7: 0x89B6E0
    bool IsA(Symbol type) const override;  // slot 8: 0x89B700
    Component* AsComponent() override;     // slot 9: 0x89B730
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x89B740
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x89B920
    ComMetaData& _GetMetaData() override;       // slot 23: 0x89B930
    // Slots 41-42: also add and remove the entry's display text. Not
    // reconstructed.
    unsigned int _AddEntryHelper(DataArray* msg) override;          // 0x89B480
    unsigned int _RemoveEntryHelper(unsigned long index) override;  // 0x89B580
    // Slot 43: shows the entry's display text, or "<display text not set
    // for file %s>". Not reconstructed: RndTextCom is not modelled.
    void _UpdateCurrent() override;  // 0x89A400
    // Slot 44: the path a symbol, resource path or string node names.
    ResourcePath _DataNodeToValue(const DataNode& node) override;  // 0x89A540

    // Registers the class (InitComponentClass). Inline in the map; emitted
    // in ui/UI.o.
    static void Init();  // 0x893F60
    // The class factory, emitted in ui/UI.o. The placement factory beside
    // it (0x896970) is InitComponentClass's.
    static Component* _Create();  // 0x896880
    // Registers the combo box's properties, limits the entries to
    // resources, then registers "display_list" and
    // "current_display_value", and describes the class.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x89A5C0

    static Symbol sId;                  // 0x1AC8278, "UIComboBoxFile"
    // Also "UIComboBoxFile". Name not in the reference map.
    static Symbol sClassName;           // 0x1AC8280
    static PropRegistry sPropRegistry;  // 0x1AC8290
    static ComMetaData sMetaData;       // 0x1AC8330

    // "display_list": the display text of each entry. Name not in the
    // reference map.
    PropArray<Symbol> mDisplayList;
};

static_assert(offsetof(UIComboBoxFileCom, mDisplayList) == 104);
static_assert(sizeof(UIComboBoxFileCom) == 144);
