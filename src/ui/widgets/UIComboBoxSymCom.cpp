// ui/UIComboBoxSymCom.o (0x89BAB0 to 0x89C8AF). The registration, the
// factory and the base class's members are emitted in ui/UI.o.
#include "ui/widgets/UIComboBoxSymCom.h"

#include <new>

// The object's statics, in the order of its static initializer (0x89C7E0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol UIComboBoxSymCom::sId("UIComboBoxSymbol");
Symbol UIComboBoxSymCom::sClassName("UIComboBoxSymbol");
PropRegistry UIComboBoxSymCom::sPropRegistry;
ComMetaData UIComboBoxSymCom::sMetaData;

// Reconstructed from eboot.elf at 0x89BB60.
Symbol UIComboBoxSymCom::_DataNodeToValue(const DataNode& node) {
    if (node.mType == kDataResourcePath || node.mType == kDataSymbol) {
        return node.Sym(nullptr);
    }
    return Symbol(node.Str(nullptr));
}

// Reconstructed from eboot.elf at 0x89BBD0. The combo box's properties
// come first (UIComboBoxBaseCom<Symbol>::_InitComboBox, 0x89BC80).
void UIComboBoxSymCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    _InitComboBox<UIComboBoxSymCom>(registry, metadata);
    metadata.mDescription = "Combo Box of symbols";
    metadata.mAuthor = "Chris McCarthy";
}

// Reconstructed from eboot.elf at 0x89C460. The deleting destructor is at
// 0x89C4D0.
UIComboBoxSymCom::~UIComboBoxSymCom() {}

// Reconstructed from eboot.elf at 0x89C550.
Symbol UIComboBoxSymCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x89C560.
Symbol UIComboBoxSymCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x89C570.
int UIComboBoxSymCom::CurrentRev() const {
    return const_cast<UIComboBoxSymCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x89C590.
bool UIComboBoxSymCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x89C5C0.
Component* UIComboBoxSymCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x89C5D0. The copy goes at the buffer's
// next 8-byte boundary.
char* UIComboBoxSymCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIComboBoxSymCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIComboBoxSymCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x89C760.
PropRegistry& UIComboBoxSymCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x89C770.
ComMetaData& UIComboBoxSymCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x897BD0. The binary emits the factory
// in ui/UI.o.
Component* UIComboBoxSymCom::_Create() {
    return new UIComboBoxSymCom();
}

// Reconstructed from eboot.elf at 0x8944B0. The binary emits the
// registration in ui/UI.o.
void UIComboBoxSymCom::Init() {
    InitComponentClass<UIComboBoxSymCom>();
}
