// render/RndSplineOptionsCom.o (0x659180 to 0x65A5BB).
#include "render/options/RndSplineOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x65A4F0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndSplineOptionsCom::sId("SplineOptions");
Symbol RndSplineOptionsCom::sClassName("SplineOptions");
PropRegistry RndSplineOptionsCom::sPropRegistry;
ComMetaData RndSplineOptionsCom::sMetaData;
RndSplineOptionsCom* theRndSplineOpts;

// Reconstructed from eboot.elf at 0x659180.
void RndSplineOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndSplineOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x659190.
bool RndSplineOptionsCom::_OnResourcesLoaded() {
    theRndSplineOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x6591A0.
RndSplineOptionsCom::RndSplineOptionsCom()
    : mShowSplines(true),
      mShowSelectedSplineControlPoints(true),
      mShowSelectedSplineControlPointsIndices(false),
      mShowSplineHulls(1),
      mShowSelectedSplineContourPoints(true),
      mShowSelectedSplineContourPointsIndices(false),
      mDepthTestSplines(true) {}

// Reconstructed from eboot.elf at 0x6591E0. The deleting destructor is at
// 0x6591F0.
RndSplineOptionsCom::~RndSplineOptionsCom() {}

// Reconstructed from eboot.elf at 0x659D00.
Symbol RndSplineOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x659D10.
Symbol RndSplineOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x659D20.
int RndSplineOptionsCom::CurrentRev() const {
    return const_cast<RndSplineOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x659D40.
bool RndSplineOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x659D70.
Component* RndSplineOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x659D80. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndSplineOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSplineOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSplineOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x659ED0.
PropRegistry& RndSplineOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x659EE0.
ComMetaData& RndSplineOptionsCom::_GetMetaData() {
    return sMetaData;
}
