// render/RndTextOptionsCom.o (0x67C560 to 0x67D42B).
#include "render/options/RndTextOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x67D360).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndTextOptionsCom::sId("TextOptions");
Symbol RndTextOptionsCom::sClassName("TextOptions");
PropRegistry RndTextOptionsCom::sPropRegistry;
ComMetaData RndTextOptionsCom::sMetaData;
RndTextOptionsCom* theRndTextOpts;

// Reconstructed from eboot.elf at 0x67C560.
void RndTextOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndTextOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x67C570.
bool RndTextOptionsCom::_OnResourcesLoaded() {
    theRndTextOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x67C580.
RndTextOptionsCom::RndTextOptionsCom()
    : mShowSelectedTextBounds(true),
      mShowSelectedTextGlyphBounds(false),
      mTextBoundsColor(Hmx::Color::GetCyan()),
      mDisplayMode(0),
      mAlignment(1),
      mJustification(1),
      mStyleSize(0) {}

// Reconstructed from eboot.elf at 0x67C610. The deleting destructor is at
// 0x67C620.
RndTextOptionsCom::~RndTextOptionsCom() {}

// Reconstructed from eboot.elf at 0x67D170.
Symbol RndTextOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x67D180.
Symbol RndTextOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x67D190.
int RndTextOptionsCom::CurrentRev() const {
    return const_cast<RndTextOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x67D1B0.
bool RndTextOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x67D1E0.
Component* RndTextOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x67D1F0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndTextOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndTextOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndTextOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x67D340.
PropRegistry& RndTextOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x67D350.
ComMetaData& RndTextOptionsCom::_GetMetaData() {
    return sMetaData;
}
