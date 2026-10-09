// render/RndOptionsCom.o (0x46A870 to 0x46ACC6).
#include "render/options/RndOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "os/memory/MemMgr.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x46AC00).
// The three ints it first sets (0x1A869C0) come from a shared header and are
// not modelled.
Symbol RndOptionsCom::sId("Options");
Symbol RndOptionsCom::sClassName("Options");
PropRegistry RndOptionsCom::sPropRegistry;
ComMetaData RndOptionsCom::sMetaData;

// Reconstructed from eboot.elf at 0x46A870.
RndOptionsCom::RndOptionsCom() {}

// Reconstructed from eboot.elf at 0x46A8B0. The deleting destructor is at
// 0x46A8C0.
RndOptionsCom::~RndOptionsCom() {}

// Reconstructed from eboot.elf at 0x46A8E0.
void RndOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    static_cast<void>(registry);
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mDescription = "Base class for all RndOptions components";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    metadata.mLightweight = true;
    metadata.mEditorRestrictions = 12;
}

// Reconstructed from eboot.elf at 0x3BF4C0. The heap index is looked up
// once.
void RndOptionsCom::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("RndOptionsCom", sMetaData);
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

// Reconstructed from eboot.elf at 0x46AA40.
Symbol RndOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x46AA50.
Symbol RndOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x46AA60.
int RndOptionsCom::CurrentRev() const {
    return const_cast<RndOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x46AA80.
bool RndOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x46AAB0.
Component* RndOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x46AAC0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x46ABE0.
PropRegistry& RndOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x46ABF0.
ComMetaData& RndOptionsCom::_GetMetaData() {
    return sMetaData;
}
