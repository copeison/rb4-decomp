// ui/UIScrollbarCom.o (0x8CD200 to 0x8CDAEF). The registration and the
// factory are emitted in ui/UI.o.
#include "ui/widgets/UIScrollbarCom.h"

#include <new>

#include "anim/drivers/AnimDriverCom.h"
#include "anim/props/PropAnimResource.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndEntityResource.h"

namespace {

// The accessors of "current_position" (0x8CDA00, 0x8CDA10). Names not in
// the reference map.
float GetCurrentPosition(const PropAccessorArgs& args) {
    return static_cast<const UIScrollbarCom*>(args.mComponent)->mCurrentPosition;
}

void SetCurrentPosition(const PropAccessorArgs& args) {
    static_cast<UIScrollbarCom*>(args.mComponent)
        ->_UpdateCurPos(*static_cast<const float*>(args.mValue));
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8CDA30).
// The int it first sets to -1 (0x1AC9BA0) comes from a shared header and is
// not modelled.
Symbol UIScrollbarCom::sId("UIScrollbar");
Symbol UIScrollbarCom::sClassName("UIScrollbar");
PropRegistry UIScrollbarCom::sPropRegistry;
ComMetaData UIScrollbarCom::sMetaData;

// Reconstructed from eboot.elf at 0x8CD200.
UIScrollbarCom::UIScrollbarCom() : mPositionAnim(), mCurrentPosition(0.0F) {}

// Reconstructed from eboot.elf at 0x8CD240. The deleting destructor is at
// 0x8CD250.
UIScrollbarCom::~UIScrollbarCom() {}

// Reconstructed from eboot.elf at 0x8CD270. Allowed in renderer entities as
// their instance component, on the root object.
void UIScrollbarCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Com that handles everything in scrollbar widget";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndEntityResource::Id());
    metadata.mInterface = Symbol("UIScrollbar");
    metadata.mInstanceCom = true;
    metadata.mRootOnly = true;
    UIScrollbarCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    ResourcePathMetadata& positionAnim = TypeSpecificMetadata(
        registry.RegisterProp(
            "position_anim", offsetof(UIScrollbarCom, mPositionAnim), kPropertyResourcePath, 8),
        prototype.mPositionAnim);
    positionAnim.mDefault = prototype.mPositionAnim;
    static_cast<void>(Symbol("prop"));
    positionAnim.mHelp = String("Which prop anim to use to position the thumb.");
    positionAnim.mResourceId = PropAnimResource::Id();

    const float positionType = 0.0F;
    FloatMetadata& position = TypeSpecificMetadata(
        registry.RegisterProp("current_position", -1, kPropertyFloat, 0), positionType);
    static_cast<void>(Symbol("prop"));
    position.mHelp = String("Current position of the focused element (normalized float).");
    position.mHistory = false;
    position.mInstanceAccess = PropMetadata::kInstanceAccessSet;
    position.mMin = 0.0F;
    position.mMax = 1.0F;
    position.mGetter = GetCurrentPosition;
    position.mSetter = SetCurrentPosition;
}

// Reconstructed from eboot.elf at 0x8CD610.
void UIScrollbarCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(AnimDriverCom::sClassName);
}

// Reconstructed from eboot.elf at 0x8CD6D0.
bool UIScrollbarCom::_OnResourcesLoaded() {
    _UpdateCurPos(mCurrentPosition);
    return true;
}

// Reconstructed from eboot.elf at 0x8CD830.
Symbol UIScrollbarCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8CD840.
Symbol UIScrollbarCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8CD850.
int UIScrollbarCom::CurrentRev() const {
    return const_cast<UIScrollbarCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8CD870.
bool UIScrollbarCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8CD8A0.
Component* UIScrollbarCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8CD8B0. The copy goes at the buffer's
// next 8-byte boundary.
char* UIScrollbarCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIScrollbarCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIScrollbarCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8CD9E0.
PropRegistry& UIScrollbarCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8CD9F0.
ComMetaData& UIScrollbarCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x8951B0. The binary emits the factory
// in ui/UI.o.
Component* UIScrollbarCom::_Create() {
    return new UIScrollbarCom();
}

// Reconstructed from eboot.elf at 0x8932B0. The binary emits the
// registration in ui/UI.o.
void UIScrollbarCom::Init() {
    InitComponentClass<UIScrollbarCom>();
}
