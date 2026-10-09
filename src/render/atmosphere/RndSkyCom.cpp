// The sky component's object (0x453030 to 0x453BFF), after
// RndShaderFogDeferred's. The reference map has no such object; the file
// is named after the class.
#include "render/atmosphere/RndSkyCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "math/color/Color.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/scene/RndSceneCom.h"
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
