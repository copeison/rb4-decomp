// ui/UIListItemCom.o (0x8C7B60 to 0x8C843F). The registration and the
// factory are emitted in ui/UI.o.
#include "ui/lists/UIListItemCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndEntityResource.h"

namespace {

// The property accessors _Init installs (0x8C8340 to 0x8C8370). Names not
// in the reference map.
int GetRelativeToFocus(const PropAccessorArgs& args) {
    return static_cast<const UIListItemCom*>(args.mComponent)->mRelativeToFocus;
}

void SetRelativeToFocusProp(const PropAccessorArgs& args) {
    static_cast<UIListItemCom*>(args.mComponent)->mRelativeToFocus =
        *static_cast<const UIListItemCom::RelativeToFocus*>(args.mValue);
}

bool GetParentListScrolling(const PropAccessorArgs& args) {
    return static_cast<const UIListItemCom*>(args.mComponent)->mParentListScrolling;
}

void SetParentListScrolling(const PropAccessorArgs& args) {
    static_cast<UIListItemCom*>(args.mComponent)->mParentListScrolling =
        *static_cast<const bool*>(args.mValue);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8C8380).
// The int it first sets to -1 (0x1AC9808) comes from a shared header and is
// not modelled.
Symbol UIListItemCom::sId("UIListItem");
Symbol UIListItemCom::sClassName("UIListItem");
PropRegistry UIListItemCom::sPropRegistry;
ComMetaData UIListItemCom::sMetaData;

// Reconstructed from eboot.elf at 0x8C7B60.
UIListItemCom::UIListItemCom() {}

// Reconstructed from eboot.elf at 0x8C7BA0. The deleting destructor is at
// 0x8C7BB0.
UIListItemCom::~UIListItemCom() {}

// Reconstructed from eboot.elf at 0x8C7BD0.
void UIListItemCom::SetRelativeToFocus(const RelativeToFocus& relative) {
    mRelativeToFocus = relative;
}

// Reconstructed from eboot.elf at 0x8C7BE0.
void UIListItemCom::SetParentListIsScrolling(bool scrolling) {
    mParentListScrolling = scrolling;
}

// Reconstructed from eboot.elf at 0x8C7BF0.
void UIListItemCom::SetDataIndex(long index) {
    mDataIndex = index;
}

// Reconstructed from eboot.elf at 0x8C7C00. Allowed in renderer entities,
// on the root object. Both properties have no storage and are not recorded
// in the editor's history.
void UIListItemCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Com that handles interface between UIList and its list items";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndEntityResource::Id());
    metadata.mRootOnly = true;
    UIListItemCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    const int relativeType = 0;
    IntMetadata<int>& relative = TypeSpecificMetadata(
        registry.RegisterProp("relative_to_focus", -1, kPropertyInt, 0), relativeType);
    static_cast<void>(Symbol("prop"));
    relative.mHelp = String("Is this list item above, below or at the current focus item?");
    relative.mHistory = false;
    relative.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kRelativeToFocusAt, String("At"), String("")},
        AllowedValue<int>{kRelativeToFocusAbove, String("Above"), String("")},
        AllowedValue<int>{kRelativeToFocusBelow, String("Below"), String("")}};
    relative.mGetter = GetRelativeToFocus;
    relative.mSetter = SetRelativeToFocusProp;

    const bool scrollingType = false;
    BoolMetadata& scrolling = TypeSpecificMetadata(
        registry.RegisterProp("parent_list_scrolling", -1, kPropertyBool, 0), scrollingType);
    static_cast<void>(Symbol("prop"));
    scrolling.mHelp = String("Indicates if the parent list is currently scrolling.");
    scrolling.mHistory = false;
    scrolling.mGetter = GetParentListScrolling;
    scrolling.mSetter = SetParentListScrolling;
}

// Reconstructed from eboot.elf at 0x8C8170.
Symbol UIListItemCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8C8180.
Symbol UIListItemCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8C8190.
int UIListItemCom::CurrentRev() const {
    return const_cast<UIListItemCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8C81B0.
bool UIListItemCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8C81E0.
Component* UIListItemCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8C81F0. The copy goes at the buffer's
// next 8-byte boundary with the list state reset.
char* UIListItemCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIListItemCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIListItemCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8C8320.
PropRegistry& UIListItemCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8C8330.
ComMetaData& UIListItemCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x895160. The binary emits the factory
// in ui/UI.o.
Component* UIListItemCom::_Create() {
    return new UIListItemCom();
}

// Reconstructed from eboot.elf at 0x892F00. The binary emits the
// registration in ui/UI.o.
void UIListItemCom::Init() {
    InitComponentClass<UIListItemCom>();
}
