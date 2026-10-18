// ui/UIListDtaProviderCom.o (0x8C6610 to 0x8C750F). The registration and
// the factory are emitted in ui/UI.o.
#include "ui/lists/UIListDtaProviderCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "ui/lists/UIListCom.h"

// The object's statics, in the order of its static initializer (0x8C7450).
// The int it first sets to -1 (0x1AC9348) comes from a shared header and is
// not modelled.
Symbol UIListDtaProviderCom::sId("UIListDtaProvider");
Symbol UIListDtaProviderCom::sClassName("UIListDtaProvider");
PropRegistry UIListDtaProviderCom::sPropRegistry;
ComMetaData UIListDtaProviderCom::sMetaData;

// Reconstructed from eboot.elf at 0x8C6610.
UIListDtaProviderCom::UIListDtaProviderCom()
    : mData(), mList(nullptr), mDta(), mDtaPath() {}

// Reconstructed from eboot.elf at 0x8C6680. The deleting destructor is at
// 0x8C6740.
UIListDtaProviderCom::~UIListDtaProviderCom() {}

// Reconstructed from eboot.elf at 0x8C6760.
void UIListDtaProviderCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription =
        "Useful for protoyping UI lists before a coder writes a real provider backed by data.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    UIListDtaProviderCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    ResourcePathMetadata& dtaPath = TypeSpecificMetadata(
        registry.RegisterProp(
            "dta_path", offsetof(UIListDtaProviderCom, mDtaPath), kPropertyResourcePath, 8),
        prototype.mDtaPath);
    dtaPath.mDefault = prototype.mDtaPath;
    static_cast<void>(Symbol("prop"));
    dtaPath.mHelp = String("DTA provider file.");
    dtaPath.mResourceId = DataResource::Id();
}

// Reconstructed from eboot.elf at 0x8C6A60. The load is
// Resource::GetOrLoad<DataResource> (0x8C6B00).
bool UIListDtaProviderCom::_OnResourcesLoaded() {
    if (mDtaPath.mPath != Symbol()) {
        mDta = Resource::GetOrLoad<DataResource>(mDtaPath, false);
    }
    _ParseDta();
    return true;
}

// Reconstructed from eboot.elf at 0x8C7090.
void UIListDtaProviderCom::_Enter() {}

// Reconstructed from eboot.elf at 0x8C70A0.
void UIListDtaProviderCom::_EditPoll() {
    if (mDta != nullptr && (mDta->mFileChangedOnDisk || mDta->NeedsReload())) {
        LoadResources(false);
    }
    _InstallSelf();
}

// Reconstructed from eboot.elf at 0x8C7160.
void UIListDtaProviderCom::_Poll() {
    _InstallSelf();
}

// Reconstructed from eboot.elf at 0x8C71E0.
void UIListDtaProviderCom::_InstallSelf() {
    if (mList == nullptr) {
        mList = mObject->GetCom<UIListCom>();
        if (mList == nullptr) {
            return;
        }
    }
    if (mList->mRuntime.mData == nullptr) {
        mList->SetData(&mData);
    }
}

// Reconstructed from eboot.elf at 0x8C7260.
Symbol UIListDtaProviderCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8C7270.
Symbol UIListDtaProviderCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8C7280.
int UIListDtaProviderCom::CurrentRev() const {
    return const_cast<UIListDtaProviderCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8C72A0.
bool UIListDtaProviderCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8C72D0.
Component* UIListDtaProviderCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8C72E0. The copy goes at the buffer's
// next 8-byte boundary.
char* UIListDtaProviderCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<UIListDtaProviderCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) UIListDtaProviderCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8C7430.
PropRegistry& UIListDtaProviderCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8C7440.
ComMetaData& UIListDtaProviderCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x895110. The binary emits the factory
// in ui/UI.o.
Component* UIListDtaProviderCom::_Create() {
    return new UIListDtaProviderCom();
}

// Reconstructed from eboot.elf at 0x892B20. The binary emits the
// registration in ui/UI.o.
void UIListDtaProviderCom::Init() {
    InitComponentClass<UIListDtaProviderCom>();
}
