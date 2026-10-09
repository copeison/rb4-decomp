// render/RndPostProcStageShaderGraphCom.o (0x633B40 to 0x63463F).
#include "render/postprocessing/chain/RndPostProcStageShaderGraphCom.h"

#include <new>

#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/shaders/RndShaderEnums.h"
#include "utl/containers/FixedVector.h"

namespace {

// The stencil the draw is limited to: reference 0 under read mask 6, as
// the bloom composite. Names not in the reference map.
constexpr unsigned int kStageStencilMode = 2;
constexpr unsigned int kStageStencilReadMask = 6;

}  // namespace

// The object's statics, in the order of its static initializer (0x634570).
Symbol RndPostProcStageShaderGraphCom::sId("PProcShaderGraph");
Symbol RndPostProcStageShaderGraphCom::sClassName("PProcShaderGraph");
PropRegistry RndPostProcStageShaderGraphCom::sPropRegistry;
ComMetaData RndPostProcStageShaderGraphCom::sMetaData;

// Reconstructed from eboot.elf at 0x633B40.
RndPostProcStageShaderGraphCom::RndPostProcStageShaderGraphCom() {}

// Reconstructed from eboot.elf at 0x633B70. The deleting destructor is at
// 0x633B80.
RndPostProcStageShaderGraphCom::~RndPostProcStageShaderGraphCom() {}

// Reconstructed from eboot.elf at 0x633E80. The object must hold a
// material component: the lookup's result is used unchecked.
void RndPostProcStageShaderGraphCom::_DrawImpl(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) {
    static_cast<void>(camera);
    static_cast<void>(params);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Shadergraph PostProc");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());
    RndMaterialCom* material = mObject->GetCom<RndMaterialCom>();

    RndBufferCollection& buffers = *batch.mBuffers;
    RndTextureBase* const source = _GetLightAccum(buffers, batch.mTarget.mSrcLightAccum);
    RndTextureBase* const dest = _GetLightAccum(buffers, batch.mTarget.mDstLightAccum);
    const RndResourceState readState = _SceneReadState();
    FixedVector<RndResourceBarrier, 2> barriers;
    barriers.resize(2);
    barriers[0].mResource = dest;
    barriers[0].mSubresource = ~0UL;
    barriers[0].mBefore = readState;
    barriers[0].mAfter = RndResourceState::kRenderTarget;
    barriers[1].mResource = source;
    barriers[1].mSubresource = ~0UL;
    barriers[1].mBefore = RndResourceState::kRenderTarget;
    barriers[1].mAfter = readState;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    // The call also passes the context's slot counts, as the map's
    // signature (unsigned int, unsigned long const*) does.
    context._DeselectAllReadWriteTexturesImpl(1U << kShaderProgramPixel);
    context.mInputSlotLimits[kShaderProgramPixel] = 0;
    RndContext::RenderTargetParams targets;
    targets.mTargets.resize(1);
    targets.mTargets[0].mTexture = dest;
    // Not 1: the target is not cleared. The value is not modelled.
    targets.mTargets[0].mClearMode = 2;
    targets.mDepthTexture =
        buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval].mDepthStencil;
    context.SetRenderTargets(targets);

    material->mRuntimeData->SelectShader(context, batch, kShaderGeoTypeDefault);
    context.SetShaderNodeTexture(kShaderNodeTextureScene, *source);
    context._SetStencilModeImpl(kStageStencilMode, 0, kStageStencilReadMask, 0);
    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(context, quad);
    context._SetStencilModeImpl(0, 0, 0, 0);
    _SwapLightAccum(batch.mTarget);
}

// Reconstructed from eboot.elf at 0x634380.
void RndPostProcStageShaderGraphCom::_UpdateDrawTarget(RndSceneDrawTarget& target) {
    _SwapLightAccum(target);
}

// Reconstructed from eboot.elf at 0x6343A0.
Symbol RndPostProcStageShaderGraphCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6343B0.
Symbol RndPostProcStageShaderGraphCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6343C0.
int RndPostProcStageShaderGraphCom::CurrentRev() const {
    return const_cast<RndPostProcStageShaderGraphCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6343E0.
bool RndPostProcStageShaderGraphCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x634410.
Component* RndPostProcStageShaderGraphCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x634420. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndPostProcStageShaderGraphCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndPostProcStageShaderGraphCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndPostProcStageShaderGraphCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x634550.
PropRegistry& RndPostProcStageShaderGraphCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x634560.
ComMetaData& RndPostProcStageShaderGraphCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x405AE0. The binary emits the factory
// with the class's Init (0x3FA240); the placement factory at 0x405B10
// constructs in given storage.
Component* RndPostProcStageShaderGraphCom::_Create() {
    return new RndPostProcStageShaderGraphCom();
}
