// render/RndLightOptionsCom.o (0x48ED90 to 0x49005B).
#include "render/options/RndLightOptionsCom.h"

#include <new>

#include "entity/props/PropArray.h"

// The object's statics, in the order of its static initializer (0x48FF90).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndLightOptionsCom::sId("LightOptions");
Symbol RndLightOptionsCom::sClassName("LightOptions");
PropRegistry RndLightOptionsCom::sPropRegistry;
ComMetaData RndLightOptionsCom::sMetaData;
RndLightOptionsCom* theRndLightOpts;

// Reconstructed from eboot.elf at 0x48ED90.
void RndLightOptionsCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
    theRndLightOpts = nullptr;
}

// Reconstructed from eboot.elf at 0x48EDA0.
bool RndLightOptionsCom::_OnResourcesLoaded() {
    theRndLightOpts = this;
    return true;
}

// Reconstructed from eboot.elf at 0x48EDB0.
RndLightOptionsCom::RndLightOptionsCom()
    : mShowLights(true),
      mShowLightsInEntities(false),
      mOnlyShowVolumetricLights(false),
      mShowSelectedLightFalloffs(true),
      mShowSelectedClipPlaneFalloffs(false),
      mShowAllLightFalloffs(false),
      mLightFalloffOpacity(0.2F),
      mShowShadowFrusta(1),
      mShowLightProbes(true),
      mShowSelectedProbeFalloffs(true),
      mLightProbeVisualization(-1),
      mLightProbeVisSmoothness(1.0F),
      mLightProbeVisOpacity(0.75F),
      mHideFlares(false),
      mShowSelectedFlareLightSources(false),
      mShowSelectedLightBoundingVolumes(false),
      mShowSelectedLightShadowCullingPlanes(false) {}

// Reconstructed from eboot.elf at 0x48EE20. The deleting destructor is at
// 0x48EE30.
RndLightOptionsCom::~RndLightOptionsCom() {}

// Reconstructed from eboot.elf at 0x48FDA0.
Symbol RndLightOptionsCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x48FDB0.
Symbol RndLightOptionsCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x48FDC0.
int RndLightOptionsCom::CurrentRev() const {
    return const_cast<RndLightOptionsCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x48FDE0.
bool RndLightOptionsCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x48FE10.
Component* RndLightOptionsCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x48FE20. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightOptionsCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightOptionsCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightOptionsCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x48FF70.
PropRegistry& RndLightOptionsCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x48FF80.
ComMetaData& RndLightOptionsCom::_GetMetaData() {
    return sMetaData;
}
