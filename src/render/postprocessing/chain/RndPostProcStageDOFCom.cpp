// render/RndPostProcStageDOFCom.o (0x631EF0 to 0x6331AF).
#include "render/postprocessing/chain/RndPostProcStageDOFCom.h"

#include <algorithm>
#include <new>

#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/postprocessing/depth_of_field/RndCShaderDOFDiscBlur.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "utl/containers/FixedVector.h"

namespace {

// The light-accumulation buffers' state while a compute shader reads them:
// pixel and non-pixel shader resource. Name not in the reference map.
constexpr RndResourceState kComputeReadState = static_cast<RndResourceState>(0xC0);

// Every program stage. Name not in the reference map.
constexpr unsigned int kAllShaderStages = (1U << kNumShaderProgramTypes) - 1;

}  // namespace

// The object's statics, in the order of its static initializer (0x6330E0).
Symbol RndPostProcStageDOFCom::sId("PProcDepthOfField");
Symbol RndPostProcStageDOFCom::sClassName("PProcDepthOfField");
PropRegistry RndPostProcStageDOFCom::sPropRegistry;
ComMetaData RndPostProcStageDOFCom::sMetaData;

// Reconstructed from eboot.elf at 0x631EF0.
RndPostProcStageDOFCom::RndPostProcStageDOFCom()
    : mNearBlurDistance(50.0F),
      mFarBlurDistance(150.0F),
      mBlurRadius(10.0F),
      mBokehOverbrightScale(2.0F),
      mBlurFalloffFunction(1) {}

// Reconstructed from eboot.elf at 0x631F50. The deleting destructor is at
// 0x631FC0.
RndPostProcStageDOFCom::~RndPostProcStageDOFCom() {
    delete mRuntimeData.mSprites;
    mRuntimeData.mSprites = nullptr;
    delete mRuntimeData.mSpriteDrawArgs;
    mRuntimeData.mSpriteDrawArgs = nullptr;
}

// Reconstructed from eboot.elf at 0x632030.
RndPostProcStageDOFCom::RuntimeData::RuntimeData() : mSprites(nullptr), mSpriteDrawArgs(nullptr) {}

// Reconstructed from eboot.elf at 0x632050.
RndPostProcStageDOFCom::RuntimeData::~RuntimeData() {}

// Reconstructed from eboot.elf at 0x632060. The buffers are created again
// without freeing the previous ones. The description's mReserved, which
// RndComputeBuffer is not seen to read, is set to 8.
bool RndPostProcStageDOFCom::_OnResourcesLoaded() {
    RndComputeBuffer::Description desc{};
    desc.mElementSize = 28;
    desc.mNumElements = 1024;
    desc.mReserved = 8;
    desc.mFlags = 5;
    mRuntimeData.mSprites = RndComputeBuffer::New(desc);
    desc.mElementSize = 4;
    desc.mNumElements = 4;
    desc.mFlags = 8;
    mRuntimeData.mSpriteDrawArgs = RndComputeBuffer::New(desc);
    return true;
}

// Reconstructed from eboot.elf at 0x632BB0. The blur distances are
// normalized to the camera's clip range (both are 1 for an empty range)
// and clamped to [0, 1]. The disc blur reads the source buffer and writes
// the destination buffer as an unordered-access resource; the buffers are
// then swapped.
void RndPostProcStageDOFCom::_DrawImpl(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) {
    RndBufferCollection& buffers = *batch.mBuffers;
    RndCShaderDOFDiscBlur::Params blur{};
    blur.mBlurFalloffFunction = -1;
    const RndCameraSettings& settings = camera.mCamera.GetCameraSettings();
    float nearBlur = 1.0F;
    float farBlur = 1.0F;
    if (settings.mFarPlane != settings.mNearPlane) {
        const float range = settings.mFarPlane - settings.mNearPlane;
        nearBlur = (mNearBlurDistance - settings.mNearPlane) / range;
        farBlur = (mFarBlurDistance - settings.mNearPlane) / range;
    }
    blur.mNearBlur = std::min(std::max(nearBlur, 0.0F), 1.0F);
    blur.mFarBlur = std::min(std::max(farBlur, 0.0F), 1.0F);
    blur.mBlurFalloffFunction = mBlurFalloffFunction;
    blur.mBlurRadius = mBlurRadius;
    blur.mBokehOverbrightScale = mBokehOverbrightScale;
    blur.mOverbrightLuminance = 1.0F / batch.mLightMgr->mTonemappingExposure;
    blur.mUseSceneMask = params.mDrawSceneMask;

    context.SetRenderTargets(nullptr, nullptr);
    context._DeselectAllSourceTexturesImpl(kAllShaderStages);
    for (unsigned long& limit : context.mOutputSlotLimits) {
        limit = 0;
    }

    FixedVector<RndResourceBarrier, 2> barriers;
    barriers.resize(2);
    barriers[0].mResource = _GetLightAccum(buffers, batch.mTarget.mDstLightAccum);
    barriers[0].mSubresource = ~0UL;
    barriers[0].mBefore = kComputeReadState;
    barriers[0].mAfter = RndResourceState::kUnorderedAccess;
    barriers[1].mResource = _GetLightAccum(buffers, batch.mTarget.mSrcLightAccum);
    barriers[1].mSubresource = ~0UL;
    barriers[1].mBefore = RndResourceState::kRenderTarget;
    barriers[1].mAfter = kComputeReadState;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    TheRndDevice()->mShaderMgr.mDOFDiscBlurCShader->Dispatch(
        context, buffers, batch.mTarget, *mRuntimeData.mSprites, blur);

    context._DeselectAllSourceTexturesImpl(1U << kShaderProgramCompute);
    context.mOutputSlotLimits[kShaderProgramCompute] = 0;
    barriers.resize(1);
    barriers[0].mBefore = RndResourceState::kUnorderedAccess;
    barriers[0].mAfter = RndResourceState::kRenderTarget;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    _SwapLightAccum(batch.mTarget);
}

// Reconstructed from eboot.elf at 0x632EC0.
Symbol RndPostProcStageDOFCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x632ED0.
Symbol RndPostProcStageDOFCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x632EE0.
int RndPostProcStageDOFCom::CurrentRev() const {
    return const_cast<RndPostProcStageDOFCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x632F00.
bool RndPostProcStageDOFCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x632F30.
Component* RndPostProcStageDOFCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x632F40. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndPostProcStageDOFCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndPostProcStageDOFCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndPostProcStageDOFCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x633080.
PropRegistry& RndPostProcStageDOFCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x633090.
ComMetaData& RndPostProcStageDOFCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x405A40. The binary emits the factory
// with the class's Init (0x3F9AD0); the placement factory at 0x405A70
// constructs in given storage.
Component* RndPostProcStageDOFCom::_Create() {
    return new RndPostProcStageDOFCom();
}
