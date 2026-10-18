// The sky component's object (0x453030 to 0x453BFF), after
// RndShaderFogDeferred's. The reference map has no such object; the file
// is named after the class.
#include "render/atmosphere/RndSkyCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "math/color/Color.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "utl/containers/FixedVector.h"

// The object's statics, in the order of its static initializer (0x453B30).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndSkyCom::sId("Sky");
Symbol RndSkyCom::sClassName("Sky");
PropRegistry RndSkyCom::sPropRegistry;
ComMetaData RndSkyCom::sMetaData;

// Reconstructed from eboot.elf at 0x453030. The deleting destructor is at
// 0x453040.
RndSkyCom::~RndSkyCom() {}

// Reconstructed from eboot.elf at 0x453060. The sky draws its object's
// material.
void RndSkyCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Material-based sky (ie. background color)";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    metadata.mRequiredComponents.push_back(RndMaterialCom::sClassName);
    RndSkyCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    PropInfo& resolutionInfo =
        registry.RegisterProp("texture_resolution", offsetof(RndSkyCom, mTextureResolution), kPropertyInt, 4);
    IntMetadata<int>& resolution = static_cast<IntMetadata<int>&>(*resolutionInfo.mMetadata.Get());
    resolution.mDefault = prototype.mTextureResolution;
    static_cast<void>(Symbol("prop"));
    resolution.mHelp =
        String("What size should be used for the sky texture? (relative to the full screen resolution)");
    resolution.mAllowedValues = eastl::vector<AllowedValue<int>>{
        {kResolutionFull, String("Full"), String("Full screen resolution")},
        {kResolutionHalf, String("Half"), String("Half screen resolution")},
        {kResolutionQuarter, String("Quarter"), String("Quarter screen resolution")},
        {kResolutionEighth, String("1/8"), String("1/8 screen resolution")},
    };
}

// Reconstructed from eboot.elf at 0x453630.
RndTextureBase* RndSkyCom::GetAtmosphereTexture(
    const RndBufferCollection& buffers) const {
    return buffers.mAtmosphere[mTextureResolution];
}

// Reconstructed from eboot.elf at 0x453640. The material must exist. With
// tiled lighting the compute passes read the texture too.
void RndSkyCom::UpdateTexture(
    RndContext& context,
    const RndSceneBatchContext& batch) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Update Sky Texture");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndMaterialCom* const material = mObject->GetCom<RndMaterialCom>();
    RndTextureBase* const texture =
        batch.mBuffers->mAtmosphere[mTextureResolution];
    const RndResourceState readState =
        TheRndDevice()->mSettings->mUseTiledLighting
        ? RndResourceState::kAllShaderResource
        : RndResourceState::kPixelShaderResource;
    FixedVector<RndResourceBarrier, 1> barriers;
    RndResourceBarrier barrier;
    barrier.mResource = texture;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = readState;
    barrier.mAfter = RndResourceState::kRenderTarget;
    barriers.push_back(barrier);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    context.SetRenderTargets(texture, nullptr);
    material->mRuntimeData->SelectShader(
        context, batch, static_cast<RndShaderGeoType>(0));
    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(context, quad);

    barriers[0].mBefore = RndResourceState::kRenderTarget;
    barriers[0].mAfter = readState;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
}

// Reconstructed from eboot.elf at 0x4538D0.
void RndSkyCom::_PostCreate() {
    RndSceneCom* scene =
        mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    if (scene != nullptr && scene->GetSky() == nullptr) {
        scene->SetSky(this);
    }
}

// Reconstructed from eboot.elf at 0x453960.
Symbol RndSkyCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x453970.
Symbol RndSkyCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x453980.
int RndSkyCom::CurrentRev() const {
    return const_cast<RndSkyCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4539A0.
bool RndSkyCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4539D0.
Component* RndSkyCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4539E0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndSkyCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndSkyCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndSkyCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x453B10.
PropRegistry& RndSkyCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x453B20.
ComMetaData& RndSkyCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4057D0. The binary emits the factory
// with the class's Init (0x3F83A0); the placement factory at 0x405800
// constructs in given storage.
Component* RndSkyCom::_Create() {
    return new RndSkyCom();
}

// Reconstructed from eboot.elf at 0x3F83A0. The binary emits the
// registration in render/RndInit.o.
void RndSkyCom::Init() {
    InitComponentClass<RndSkyCom>();
}
