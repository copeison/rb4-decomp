// render/RndOverlayOptionsCom.o (0x5F9880 to 0x5FA690).
#include "render/debug/RndOverlayOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"
#include "render/options/RndOptions.h"

// The object's statics, in the order of its static initializer (0x5FA5E0).
// The three ints it first sets (0x1AA7CA8) come from a shared header and are
// not modelled.
Symbol RndOverlayOptionsCom::sId("OverlayOptions");
Symbol RndOverlayOptionsCom::sClassName("OverlayOptions");
PropRegistry RndOverlayOptionsCom::sPropRegistry;
ComMetaData RndOverlayOptionsCom::sMetaData;
RndOverlayOptionsCom* theRndOverlayOpts;

// Reconstructed from eboot.elf at 0x5F9880.
void RndOverlayOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndOverlayOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x5F9890.
bool RndOverlayOptionsCom::_OnResourcesLoaded() {
    theRndOverlayOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x5F98A0.
RndOverlayOptionsCom::RndOverlayOptionsCom() : mOverlaysChanged(false) {}

// Reconstructed from eboot.elf at 0x5F98D0. The deleting destructor is at
// 0x5F98E0.
RndOverlayOptionsCom::~RndOverlayOptionsCom() {}

// Reconstructed from eboot.elf at 0x5F9900. The inherited registry comes
// from a temporary component, whose _GetPropRegistry returns sPropRegistry.
void RndOverlayOptionsCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndOptionsCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Options controlling rendering overlays";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndOptionsResource::Id());
    RndOverlayOptionsCom component;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, component._GetPropRegistry());
    }
}

// Reconstructed from eboot.elf at 0x5F9AB0.
PropRegistry& RndOverlayOptionsCom::_GetPropRegistry() {
    if (mOverlaysChanged) {
        _RebuildRegistry();
    }
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x5FA2A0.
Symbol RndOverlayOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x5FA2B0.
Symbol RndOverlayOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x5FA2C0.
int RndOverlayOptionsCom::CurrentRev() const {
    return const_cast<RndOverlayOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x5FA2E0.
bool RndOverlayOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5FA310.
Component* RndOverlayOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x5FA320. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndOverlayOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndOverlayOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndOverlayOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x5FA460.
ComMetaData& RndOverlayOptionsCom::_GetMetaData() {
    return sMetaData;
}
