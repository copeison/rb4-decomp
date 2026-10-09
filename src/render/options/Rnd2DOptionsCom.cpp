// render/Rnd2DOptionsCom.o (0x6D1910 to 0x6D1E6B).
#include "render/options/Rnd2DOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x6D1DA0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol Rnd2DOptionsCom::sId("2DOptions");
Symbol Rnd2DOptionsCom::sClassName("2DOptions");
PropRegistry Rnd2DOptionsCom::sPropRegistry;
ComMetaData Rnd2DOptionsCom::sMetaData;
Rnd2DOptionsCom* theRnd2DOpts;

// Reconstructed from eboot.elf at 0x6D1910.
void Rnd2DOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRnd2DOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x6D1920.
bool Rnd2DOptionsCom::_OnResourcesLoaded() {
    theRnd2DOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x6D1930.
Rnd2DOptionsCom::Rnd2DOptionsCom()
    : mShowSelectedScale9Vertices(true) {}

// Reconstructed from eboot.elf at 0x6D1960. The deleting destructor is at
// 0x6D1970.
Rnd2DOptionsCom::~Rnd2DOptionsCom() {}

// Reconstructed from eboot.elf at 0x6D1BC0.
Symbol Rnd2DOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6D1BD0.
Symbol Rnd2DOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6D1BE0.
int Rnd2DOptionsCom::CurrentRev() const {
    return const_cast<Rnd2DOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6D1C00.
bool Rnd2DOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6D1C30.
Component* Rnd2DOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6D1C40. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* Rnd2DOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<Rnd2DOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) Rnd2DOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6D1D80.
PropRegistry& Rnd2DOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6D1D90.
ComMetaData& Rnd2DOptionsCom::_GetMetaData() {
    return sMetaData;
}
