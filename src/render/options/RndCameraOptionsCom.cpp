// render/RndCameraOptionsCom.o (0x6B9270 to 0x6B993B).
#include "render/options/RndCameraOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x6B9870).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndCameraOptionsCom::sId("CameraOptions");
Symbol RndCameraOptionsCom::sClassName("CameraOptions");
PropRegistry RndCameraOptionsCom::sPropRegistry;
ComMetaData RndCameraOptionsCom::sMetaData;
RndCameraOptionsCom* theRndCameraOpts;

// Reconstructed from eboot.elf at 0x6B9270.
void RndCameraOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndCameraOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x6B9280.
bool RndCameraOptionsCom::_OnResourcesLoaded() {
    theRndCameraOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x6B9290.
RndCameraOptionsCom::RndCameraOptionsCom()
    : mShowCameras(true),
      mShowSelectedCameraFrusta(true),
      mCameraFrustaOpacity(0.3F) {}

// Reconstructed from eboot.elf at 0x6B92D0. The deleting destructor is at
// 0x6B92E0.
RndCameraOptionsCom::~RndCameraOptionsCom() {}

// Reconstructed from eboot.elf at 0x6B9680.
Symbol RndCameraOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6B9690.
Symbol RndCameraOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6B96A0.
int RndCameraOptionsCom::CurrentRev() const {
    return const_cast<RndCameraOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6B96C0.
bool RndCameraOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6B96F0.
Component* RndCameraOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x6B9700. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndCameraOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndCameraOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndCameraOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6B9850.
PropRegistry& RndCameraOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6B9860.
ComMetaData& RndCameraOptionsCom::_GetMetaData() {
    return sMetaData;
}
