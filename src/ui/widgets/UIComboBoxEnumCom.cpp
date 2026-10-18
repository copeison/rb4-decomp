// ui/UIComboBoxEnumCom.o (0x898F30 to 0x89A3FF). The registration, the
// factory and the base class's members are emitted in ui/UI.o.
#include "ui/widgets/UIComboBoxEnumCom.h"

#include <new>

namespace {

// The accessor of "current_display_value" (0x89A2A0): the current entry's
// display text, or an empty symbol. Name not in the reference map.
Symbol GetCurrentDisplayValue(const PropAccessorArgs& args) {
    const UIComboBoxEnumCom& box = *static_cast<const UIComboBoxEnumCom*>(args.mComponent);
    Symbol text;
    if (box.mCurrentIndex != UIComboBoxEnumCom::kNoIndex) {
        const unsigned long index =
            box.mCurrentIndex < box.mEntries.mSize ? box.mCurrentIndex : UIComboBoxEnumCom::kNoIndex;
        if (index < box.mDisplayList.mSize) {
            text = box.mDisplayList[index];
        }
    }
    return text;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x89A330).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol UIComboBoxEnumCom::sId("UIComboBoxEnum");
Symbol UIComboBoxEnumCom::sClassName("UIComboBoxEnum");
PropRegistry UIComboBoxEnumCom::sPropRegistry;
ComMetaData UIComboBoxEnumCom::sMetaData;

// Reconstructed from eboot.elf at 0x899070.
int UIComboBoxEnumCom::_DataNodeToValue(const DataNode& node) {
    return node.Int(nullptr);
}

// Reconstructed from eboot.elf at 0x899080. The combo box's properties
// come first (UIComboBoxBaseCom<int>::_InitComboBox, 0x899490).
void UIComboBoxEnumCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    _InitComboBox<UIComboBoxEnumCom>(registry, metadata);
    UIComboBoxEnumCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    PropInfo& display = registry.RegisterPropArray(
        "display_list", offsetof(UIComboBoxEnumCom, mDisplayList), sizeof(PropArray<Symbol>));
    ArrayMetadata& displayList = static_cast<ArrayMetadata&>(*display.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(display, kPropertySymbol, sizeof(Symbol));
    displayList.mInstanceAccess = PropMetadata::kInstanceAccessSet;
    displayList.mHelp = String("The corresponding display text associated to the entries.");

    const Symbol displayType;
    SymbolMetadata& displayValue = TypeSpecificMetadata(
        registry.RegisterProp("current_display_value", -1, kPropertySymbol, 0), displayType);
    static_cast<void>(Symbol("prop"));
    displayValue.mHelp = String("The current display text of the ComboBox (read-only)");
    displayValue.mReadOnly = true;
    displayValue.mSaves = false;
    displayValue.mGetter = GetCurrentDisplayValue;
    displayValue.mHistory = false;
    displayValue.mInstanceAccess = PropMetadata::kInstanceAccessView;

    metadata.mDescription = "Combo Box of enums";
    metadata.mAuthor = "Chris McCarthy";
}

// Reconstructed from eboot.elf at 0x899D20. The deleting destructor is at
// 0x899F60.
UIComboBoxEnumCom::~UIComboBoxEnumCom() {}

// Reconstructed from eboot.elf at 0x89A020.
Symbol UIComboBoxEnumCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x89A030.
Symbol UIComboBoxEnumCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x89A040.
int UIComboBoxEnumCom::CurrentRev() const {
    return const_cast<UIComboBoxEnumCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x89A060.
bool UIComboBoxEnumCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x89A090.
Component* UIComboBoxEnumCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x89A0A0. The copy goes at the buffer's
// next 8-byte boundary.
char* UIComboBoxEnumCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIComboBoxEnumCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIComboBoxEnumCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x89A280.
PropRegistry& UIComboBoxEnumCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x89A290.
ComMetaData& UIComboBoxEnumCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x895250. The binary emits the factory
// in ui/UI.o.
Component* UIComboBoxEnumCom::_Create() {
    return new UIComboBoxEnumCom();
}

// Reconstructed from eboot.elf at 0x893A10. The binary emits the
// registration in ui/UI.o.
void UIComboBoxEnumCom::Init() {
    InitComponentClass<UIComboBoxEnumCom>();
}
