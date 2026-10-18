// ui/UIComboBoxFileCom.o (0x89A400 to 0x89BAAF). The registration, the
// factory and the base class's members are emitted in ui/UI.o.
#include "ui/widgets/UIComboBoxFileCom.h"

#include <new>

namespace {

// The accessor of "current_display_value" (0x89B940): the current entry's
// display text, or an empty symbol. Name not in the reference map.
Symbol GetCurrentDisplayValue(const PropAccessorArgs& args) {
    const UIComboBoxFileCom& box = *static_cast<const UIComboBoxFileCom*>(args.mComponent);
    Symbol text;
    if (box.mCurrentIndex != UIComboBoxFileCom::kNoIndex) {
        const unsigned long index =
            box.mCurrentIndex < box.mEntries.mSize ? box.mCurrentIndex : UIComboBoxFileCom::kNoIndex;
        if (index < box.mDisplayList.mSize) {
            text = box.mDisplayList[index];
        }
    }
    return text;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x89B9E0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol UIComboBoxFileCom::sId("UIComboBoxFile");
Symbol UIComboBoxFileCom::sClassName("UIComboBoxFile");
PropRegistry UIComboBoxFileCom::sPropRegistry;
ComMetaData UIComboBoxFileCom::sMetaData;

// Reconstructed from eboot.elf at 0x89A540.
ResourcePath UIComboBoxFileCom::_DataNodeToValue(const DataNode& node) {
    ResourcePath path;
    if (node.mType == kDataResourcePath || node.mType == kDataSymbol) {
        path = node.Sym(nullptr).Str();
    } else {
        path = node.Str(nullptr);
    }
    return path;
}

// Reconstructed from eboot.elf at 0x89A5C0. The combo box's properties
// come first (UIComboBoxBaseCom<ResourcePath>::_InitComboBox, 0x89AB30),
// and the entries are limited to resources.
void UIComboBoxFileCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    _InitComboBox<UIComboBoxFileCom>(registry, metadata);
    const ResourcePath entryType;
    ResourcePathMetadata& entry = TypeSpecificMetadata(
        static_cast<ArrayMetadata&>(
            *sPropRegistry.FindProp(PropPath(Symbol("entry_list")), nullptr)->mMetadata.Get())
            .mItemInfo,
        entryType);
    entry.mResourceId = Resource::Id();
    UIComboBoxFileCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    PropInfo& display = registry.RegisterPropArray(
        "display_list", offsetof(UIComboBoxFileCom, mDisplayList), sizeof(PropArray<Symbol>));
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

    metadata.mDescription = "Combo Box of files";
    metadata.mAuthor = "Chris McCarthy";
}

// Reconstructed from eboot.elf at 0x89B3C0. The deleting destructor is at
// 0x89B600.
UIComboBoxFileCom::~UIComboBoxFileCom() {}

// Reconstructed from eboot.elf at 0x89B6C0.
Symbol UIComboBoxFileCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x89B6D0.
Symbol UIComboBoxFileCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x89B6E0.
int UIComboBoxFileCom::CurrentRev() const {
    return const_cast<UIComboBoxFileCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x89B700.
bool UIComboBoxFileCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x89B730.
Component* UIComboBoxFileCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x89B740. The copy goes at the buffer's
// next 8-byte boundary.
char* UIComboBoxFileCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIComboBoxFileCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIComboBoxFileCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x89B920.
PropRegistry& UIComboBoxFileCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x89B930.
ComMetaData& UIComboBoxFileCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x896880. The binary emits the factory
// in ui/UI.o.
Component* UIComboBoxFileCom::_Create() {
    return new UIComboBoxFileCom();
}

// Reconstructed from eboot.elf at 0x893F60. The binary emits the
// registration in ui/UI.o.
void UIComboBoxFileCom::Init() {
    InitComponentClass<UIComboBoxFileCom>();
}
