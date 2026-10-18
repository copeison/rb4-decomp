// The "UIListEditorData" component's object (0x8C7510 to 0x8C7B5F). The
// registration and the factory are emitted in ui/UI.o.
#include "ui/lists/UIListEditorDataCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "ui/lists/UIListCom.h"

// The object's statics, in the order of its static initializer (0x8C7AA0).
// The int it first sets to -1 (0x1AC95A8) comes from a shared header and is
// not modelled.
Symbol UIListEditorDataCom::sId("UIListEditorData");
Symbol UIListEditorDataCom::sClassName("UIListEditorData");
PropRegistry UIListEditorDataCom::sPropRegistry;
ComMetaData UIListEditorDataCom::sMetaData;

// Reconstructed from eboot.elf at 0x8C7510.
void UIListEditorDataCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndEditorDrawCom::_Init(registry, metadata);
    metadata.mDescription = "RndEditorDrawCom counterpart to UIListCom";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources = UIListCom::sMetaData.mAllowedResources;
    metadata.mDefaultFor.push_back(UIListCom::sClassName);
    metadata.mRequiredComponents.push_back(UIListCom::sClassName);
}

// Reconstructed from eboot.elf at 0x8C7840.
unsigned int UIListEditorDataCom::_GetCapabilityFlagsImpl() const {
    return 0x14;
}

// Reconstructed from eboot.elf at 0x8C7850.
bool UIListEditorDataCom::_IsShownUnselectedImpl() const {
    return true;
}

// Reconstructed from eboot.elf at 0x8C7860.
bool UIListEditorDataCom::_IntersectSegmentImpl(
    const RndCameraContext& camera,
    const Segment& segment,
    float& t,
    Vector3* point) const {
    static_cast<void>(camera);
    static_cast<void>(segment);
    static_cast<void>(t);
    static_cast<void>(point);
    return false;
}

// Reconstructed from eboot.elf at 0x8C7870.
bool UIListEditorDataCom::_FrustumContainsImpl(
    const RndCameraContext& camera,
    const Frustum& frustum) const {
    static_cast<void>(camera);
    static_cast<void>(frustum);
    return false;
}

// Reconstructed from eboot.elf at 0x8C7880.
bool UIListEditorDataCom::_FrustumExcludesImpl(
    const RndCameraContext& camera,
    const Frustum& frustum) const {
    static_cast<void>(camera);
    static_cast<void>(frustum);
    return true;
}

// Reconstructed from eboot.elf at 0x8C7890. The deleting destructor is at
// 0x8C78A0.
UIListEditorDataCom::~UIListEditorDataCom() {}

// Reconstructed from eboot.elf at 0x8C78C0.
Symbol UIListEditorDataCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8C78D0.
Symbol UIListEditorDataCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8C78E0.
int UIListEditorDataCom::CurrentRev() const {
    return const_cast<UIListEditorDataCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8C7900.
bool UIListEditorDataCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8C7930.
Component* UIListEditorDataCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8C7940. The copy goes at the buffer's
// next 8-byte boundary and starts a fresh RuntimeData.
char* UIListEditorDataCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIListEditorDataCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIListEditorDataCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8C7A80.
PropRegistry& UIListEditorDataCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8C7A90.
ComMetaData& UIListEditorDataCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x898EA0. The binary emits the factory
// in ui/UI.o.
Component* UIListEditorDataCom::_Create() {
    return new UIListEditorDataCom();
}

// Reconstructed from eboot.elf at 0x894960. The binary emits the
// registration in ui/UI.o.
void UIListEditorDataCom::Init() {
    InitComponentClass<UIListEditorDataCom>();
}
