// render/RndSelectionOptionsCom.o (0x46ACD0 to 0x46B87B).
#include "render/options/RndSelectionOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x46B7B0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndSelectionOptionsCom::sId("SelectionOptions");
Symbol RndSelectionOptionsCom::sClassName("SelectionOptions");
PropRegistry RndSelectionOptionsCom::sPropRegistry;
ComMetaData RndSelectionOptionsCom::sMetaData;
RndSelectionOptionsCom* theRndSelectionOpts;

// Reconstructed from eboot.elf at 0x46ACD0.
void RndSelectionOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndSelectionOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x46ACE0.
bool RndSelectionOptionsCom::_OnResourcesLoaded() {
    theRndSelectionOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x46ACF0.
RndSelectionOptionsCom::RndSelectionOptionsCom()
    : mShowWireframe(true),
      mShowBoundingSpheres(false),
      mShowDistances(false),
      mDistanceLinesColor(Hmx::Color::GetCyan()),
      mOccludedDistanceLinesAlpha(0.25F) {}

// Reconstructed from eboot.elf at 0x46AD80. The deleting destructor is at
// 0x46AD90.
RndSelectionOptionsCom::~RndSelectionOptionsCom() {}

// Reconstructed from eboot.elf at 0x46B3C0.
Symbol RndSelectionOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x46B3D0.
Symbol RndSelectionOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x46B3E0.
int RndSelectionOptionsCom::CurrentRev() const {
    return const_cast<RndSelectionOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x46B400.
bool RndSelectionOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x46B430.
Component* RndSelectionOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x46B440. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndSelectionOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSelectionOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSelectionOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x46B590.
PropRegistry& RndSelectionOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x46B5A0.
ComMetaData& RndSelectionOptionsCom::_GetMetaData() {
    return sMetaData;
}
