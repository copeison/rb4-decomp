// render/RndMeshOptionsCom.o (0x5D0CE0 to 0x5D19FB).
#include "render/options/RndMeshOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x5D1930).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndMeshOptionsCom::sId("MeshOptions");
Symbol RndMeshOptionsCom::sClassName("MeshOptions");
PropRegistry RndMeshOptionsCom::sPropRegistry;
ComMetaData RndMeshOptionsCom::sMetaData;
RndMeshOptionsCom* theRndMeshOpts;

// Reconstructed from eboot.elf at 0x5D0CE0.
void RndMeshOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndMeshOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x5D0CF0.
bool RndMeshOptionsCom::_OnResourcesLoaded() {
    theRndMeshOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x5D0D00.
RndMeshOptionsCom::RndMeshOptionsCom()
    : mLabelSelectedMeshVertices(false),
      mSelectedMeshNormals(false),
      mSelectedMeshTangents(false),
      mMeshTangentSpaceScale(1.0F),
      mRestrictVertexRange(false),
      mStartVertex(0),
      mNumVertices(100) {}

// Reconstructed from eboot.elf at 0x5D0D50. The deleting destructor is at
// 0x5D0D60.
RndMeshOptionsCom::~RndMeshOptionsCom() {}

// Reconstructed from eboot.elf at 0x5D1540.
Symbol RndMeshOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x5D1550.
Symbol RndMeshOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x5D1560.
int RndMeshOptionsCom::CurrentRev() const {
    return const_cast<RndMeshOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x5D1580.
bool RndMeshOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5D15B0.
Component* RndMeshOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x5D15C0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndMeshOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndMeshOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndMeshOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x5D1710.
PropRegistry& RndMeshOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x5D1720.
ComMetaData& RndMeshOptionsCom::_GetMetaData() {
    return sMetaData;
}
