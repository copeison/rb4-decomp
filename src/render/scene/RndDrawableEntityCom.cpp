// The drawable entity component (0x6C05D0 to 0x6C0B7B); the reference map's
// render/RndEntityCom.o.
#include "render/scene/RndDrawableEntityCom.h"

#include <new>

#include "os/memory/MemMgr.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/textures/RndTexRendererMgr.h"

// The object's statics, in the order of its static initializer (0x6C0AB0).
// The three ints it first sets (0x1AB0094) come from a shared header and are
// not modelled.
Symbol RndDrawableEntityCom::sId("DrawableEntity");
Symbol RndDrawableEntityCom::sClassName("DrawableEntity");
PropRegistry RndDrawableEntityCom::sPropRegistry;
ComMetaData RndDrawableEntityCom::sMetaData;

// Reconstructed from eboot.elf at 0x6C05D0.
RndDrawableEntityCom::RndDrawableEntityCom() {}

// Reconstructed from eboot.elf at 0x6C0600. The class's constructor
// inlines it.
RndDrawableEntityCom::RuntimeData::RuntimeData() : mTexRendererMgr(nullptr) {}

// Reconstructed from eboot.elf at 0x6C0610. The deleting destructor is at
// 0x6C0660.
RndDrawableEntityCom::~RndDrawableEntityCom() {
    delete mRuntimeData.mTexRendererMgr;
    mRuntimeData.mTexRendererMgr = nullptr;
}

// Reconstructed from eboot.elf at 0x6C06B0.
RndTexRendererMgr* RndDrawableEntityCom::ObtainTexRendererMgr() {
    if (mRuntimeData.mTexRendererMgr == nullptr) {
        mRuntimeData.mTexRendererMgr = new RndTexRendererMgr();
    }
    return mRuntimeData.mTexRendererMgr;
}

// Reconstructed from eboot.elf at 0x6C06F0. The inherited registry comes
// from a temporary component, whose _GetPropRegistry returns sPropRegistry.
void RndDrawableEntityCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mDescription = "Goes on the root object of all drawable entities";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mEditorRestrictions = 0;
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    metadata.mInterface = sClassName;
    RndDrawableEntityCom component;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, component._GetPropRegistry());
    }
}

// Reconstructed from eboot.elf at 0x4105A0. The heap index is looked up
// once.
void RndDrawableEntityCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("RndDrawableEntityCom", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

// Reconstructed from eboot.elf at 0x6C08E0.
Symbol RndDrawableEntityCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6C08F0.
Symbol RndDrawableEntityCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6C0900.
int RndDrawableEntityCom::CurrentRev() const {
    return const_cast<RndDrawableEntityCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6C0920.
bool RndDrawableEntityCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6C0950.
Component* RndDrawableEntityCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6C0960. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndDrawableEntityCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndDrawableEntityCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndDrawableEntityCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6C0A90.
PropRegistry& RndDrawableEntityCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6C0AA0.
ComMetaData& RndDrawableEntityCom::_GetMetaData() {
    return sMetaData;
}
