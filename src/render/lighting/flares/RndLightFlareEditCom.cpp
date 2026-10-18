// render/RndLightFlareEditCom.o (0x47E780 to 0x47EEDF).
#include "render/lighting/flares/RndLightFlareEditCom.h"

#include <new>

#include "entity/core/GameObject.h"
#include "render/context/RndContext.h"
#include "render/lighting/flares/RndLightFlareCom.h"
#include "render/materials/RndMaterialCom.h"
#include "render/options/RndLightOptionsCom.h"
#include "render/queries/RndOcclusionQueryMgr.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x47EE10; see RndMeshCom.cpp). Unused here.
// Names not in the reference map.
[[maybe_unused]] unsigned int gNullObjectId = 0xFFFFFFFFU;  // 0x1A88058
[[maybe_unused]] int gMeshGroupSize2D = 8;                // 0x1A8805C
[[maybe_unused]] int gMeshGroupSize3D = 4;                // 0x1A88060

}  // namespace

// The object's statics, in the order of its static initializer (0x47EE10).
Symbol RndLightFlareEditCom::sId("LightFlareEditorData");
Symbol RndLightFlareEditCom::sClassName("LightFlareEditorData");
PropRegistry RndLightFlareEditCom::sPropRegistry;
ComMetaData RndLightFlareEditCom::sMetaData;

// Reconstructed from eboot.elf at 0x47E780.
RndLightFlareEditCom::RndLightFlareEditCom() {}

// Reconstructed from eboot.elf at 0x47E7B0. The deleting destructor is at
// 0x47E7C0.
RndLightFlareEditCom::~RndLightFlareEditCom() {}

// Reconstructed from eboot.elf at 0x47EA30.
unsigned int RndLightFlareEditCom::_GetCapabilityFlagsImpl() const {
    return 0x42;
}

// Reconstructed from eboot.elf at 0x47EA40.
bool RndLightFlareEditCom::_FilterShowingImpl(
    const RndCameraContext* camera,
    bool selected) const {
    static_cast<void>(camera);
    if (!selected) {
        return false;
    }
    const RndLightOptionsCom* options = theRndLightOpts;
    if (options->mRuntimeData.mSuppressed) {
        return false;
    }
    return options->mShowSelectedFlareLightSources;
}

// Reconstructed from eboot.elf at 0x47EA70.
void RndLightFlareEditCom::_EditorDrawImpl(
    RndContext& context,
    bool selected,
    const VectorAdapter<unsigned long>& subEditables) {
    static_cast<void>(selected);
    static_cast<void>(subEditables);
    const Sphere& sphere = mObject->GetCom<RndDrawNodeCom>()->mRuntime.mWorldSphere;
    const RndShadingMode shadingMode = context.mShadingMode;
    context.SetShadingMode(kShadingModeWireframe);
    context._SetThickLinesImpl(false);
    context.mBlendMode = RndBlendMode::kSource;
    context._SetBlendModeImpl(RndBlendMode::kSource, Hmx::Color::GetWhite());
    context._SetDepthModeImpl(1);
    context._SetCullModeImpl(kCullBack);
    TheRndDevice()->mShaderMgr.mBasicShader->Select(context, RndShaderBasic::Params());
    RndOcclusionQueryMgr::DrawOcclusionSphere(context, sphere);
    context.SetShadingMode(shadingMode);
}

// Reconstructed from eboot.elf at 0x47E7E0.
void RndLightFlareEditCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RndEditorDrawCom::_Init(registry, metadata);
    metadata.mDescription = "RndEditorDrawCom counterpart to RndLightFlareCom";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mDefaultFor.push_back(RndLightFlareCom::sClassName);
    metadata.mRequiredComponents.push_back(RndLightFlareCom::sClassName);
}

// Reconstructed from eboot.elf at 0x47EC30.
Symbol RndLightFlareEditCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x47EC40.
Symbol RndLightFlareEditCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x47EC50.
int RndLightFlareEditCom::CurrentRev() const {
    return const_cast<RndLightFlareEditCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x47EC70.
bool RndLightFlareEditCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x47ECA0.
Component* RndLightFlareEditCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x47ECB0. The copy goes at the buffer's
// next 8-byte boundary and starts a fresh RuntimeData.
char* RndLightFlareEditCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightFlareEditCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightFlareEditCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x47EDF0.
PropRegistry& RndLightFlareEditCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x47EE00.
ComMetaData& RndLightFlareEditCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4060F0. The binary emits the factory
// with the other render factories.
Component* RndLightFlareEditCom::_Create() {
    return new RndLightFlareEditCom();
}

// Reconstructed from eboot.elf at 0x3FE980. The binary emits the registration
// in render/RndInit.o.
void RndLightFlareEditCom::Init() {
    InitComponentClass<RndLightFlareEditCom>();
}
