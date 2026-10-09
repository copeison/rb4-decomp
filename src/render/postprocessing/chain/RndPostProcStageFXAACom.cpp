// render/RndPostProcStageFXAACom.o (0x6331B0 to 0x633B3F).
#include "render/postprocessing/chain/RndPostProcStageFXAACom.h"

#include <new>

#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/postprocessing/antialiasing/RndShaderFXAA.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "utl/containers/FixedVector.h"

namespace {

// The stencil the draw is limited to: reference 0 under read mask 6, as
// the bloom composite. Names not in the reference map.
constexpr unsigned int kStageStencilMode = 2;
constexpr unsigned int kStageStencilReadMask = 6;

}  // namespace

// The object's statics, in the order of its static initializer (0x633A70).
Symbol RndPostProcStageFXAACom::sId("PProcFXAA");
Symbol RndPostProcStageFXAACom::sClassName("PProcFXAA");
PropRegistry RndPostProcStageFXAACom::sPropRegistry;
ComMetaData RndPostProcStageFXAACom::sMetaData;

// Reconstructed from eboot.elf at 0x6331B0.
RndPostProcStageFXAACom::RndPostProcStageFXAACom() {}

// Reconstructed from eboot.elf at 0x6331E0. The deleting destructor is at
// 0x6331F0.
RndPostProcStageFXAACom::~RndPostProcStageFXAACom() {}

// Reconstructed from eboot.elf at 0x6333C0.
void RndPostProcStageFXAACom::_DrawImpl(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) {
    static_cast<void>(camera);
    static_cast<void>(params);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("FXAA");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

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

    RndShaderFXAA::Params fxaa;
    fxaa.mSource = source;
    fxaa.mReserved8[0] = 1.0F;
    fxaa.mReserved8[1] = 1.0F;
    TheRndDevice()->mShaderMgr.mFXAAShader->Select(context, fxaa);
    context._SetStencilModeImpl(kStageStencilMode, 0, kStageStencilReadMask, 0);
    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(context, quad);
    context._SetStencilModeImpl(0, 0, 0, 0);
    _SwapLightAccum(batch.mTarget);
}

// Reconstructed from eboot.elf at 0x633880.
void RndPostProcStageFXAACom::_UpdateDrawTarget(RndSceneDrawTarget& target) {
    _SwapLightAccum(target);
}

// Reconstructed from eboot.elf at 0x6338A0.
Symbol RndPostProcStageFXAACom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6338B0.
Symbol RndPostProcStageFXAACom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6338C0.
int RndPostProcStageFXAACom::CurrentRev() const {
    return const_cast<RndPostProcStageFXAACom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6338E0.
bool RndPostProcStageFXAACom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x633910.
Component* RndPostProcStageFXAACom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x633920. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndPostProcStageFXAACom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndPostProcStageFXAACom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndPostProcStageFXAACom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x633A50.
PropRegistry& RndPostProcStageFXAACom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x633A60.
ComMetaData& RndPostProcStageFXAACom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x405A90. The binary emits the factory
// with the class's Init (0x3F9E90); the placement factory at 0x405AC0
// constructs in given storage.
Component* RndPostProcStageFXAACom::_Create() {
    return new RndPostProcStageFXAACom();
}
