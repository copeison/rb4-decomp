// render/RndPostProcStageBloomCom.o (0x62FA00 to 0x63192F).
#include "render/postprocessing/chain/RndPostProcStageBloomCom.h"

#include <new>

#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/postprocessing/bloom/RndShaderBloom.h"
#include "render/postprocessing/blur/RndShaderBlur.h"
#include "render/postprocessing/downsample/RndShaderDownsample.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "utl/containers/FixedVector.h"

namespace {

// The downsample programs the pass selects (RndShaderDownsample's
// HX_DOWNSAMPLE type). Names not in the reference map.
constexpr int kDownsampleColor2x = 0;
constexpr int kDownsampleBloom2x = 2;
constexpr int kDownsampleBloom4x = 3;

// The blur radii, as fractions of the buffer height: the horizontal pass
// scales its width-relative radius by the buffer's aspect ratio. Names not
// in the reference map.
constexpr float kHalfSizeBlurRadius = 0.0129629625F;
constexpr float kQuarterSizeBlurRadius = 0.025925925F;
constexpr float kQuarterSizeOnlyBlurRadius = 0.014814815F;

// The stencil the composite is limited to: reference 0 under read mask 6.
// Names not in the reference map; the meaning of the mode is not
// recovered.
constexpr unsigned int kCompositeStencilMode = 2;
constexpr unsigned int kCompositeStencilReadMask = 6;

// A transition of the whole resource. Name not in the reference map.
RndResourceBarrier Transition(
    RndResourceBarrierPhase phase,
    RndShaderResource* resource,
    RndResourceState before,
    RndResourceState after) {
    RndResourceBarrier barrier;
    barrier.mPhase = phase;
    barrier.mResource = resource;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = before;
    barrier.mAfter = after;
    return barrier;
}

// Unbinds the pixel stage's read-write textures and forgets its slot
// count. The call also passes the context's slot counts, as the map's
// signature (unsigned int, unsigned long const*) does. Inlined into the
// stages. Name not in the reference map.
void DeselectPixelTextures(RndContext& context) {
    context._DeselectAllReadWriteTexturesImpl(1U << kShaderProgramPixel);
    context.mInputSlotLimits[kShaderProgramPixel] = 0;
}

// The horizontal blur radius of a buffer: the height-relative radius in
// texels of its width.
float HorizontalRadius(const RndTextureBase& texture, float radius) {
    return static_cast<float>(static_cast<int>(texture.mBaseDesc.mHeight)) /
        static_cast<float>(static_cast<int>(texture.mBaseDesc.mWidth)) * radius;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x631860).
Symbol RndPostProcStageBloomCom::sId("PProcBloom");
Symbol RndPostProcStageBloomCom::sClassName("PProcBloom");
PropRegistry RndPostProcStageBloomCom::sPropRegistry;
ComMetaData RndPostProcStageBloomCom::sMetaData;

// Reconstructed from eboot.elf at 0x62FA00.
RndPostProcStageBloomCom::RndPostProcStageBloomCom()
    : mIntensity(1.0F),
      mPower(0.45454544F),
      mShape(0.25F),
      mUseHalfSizeBuffer(true),
      mValueBased(false),
      mOverbrightHuePreservation(false),
      mOverbrightWhiteningStrength(0.25F),
      mMaxOverbrightWhitening(0.5F) {}

// Reconstructed from eboot.elf at 0x62FA60. The deleting destructor is at
// 0x62FA70.
RndPostProcStageBloomCom::~RndPostProcStageBloomCom() {}

// Reconstructed from eboot.elf at 0x6305A0. Thresholds the source into the
// first downsample buffer, blurs it horizontally into the second and back
// vertically, then (with the half-size buffer) downsamples that into the
// next level and blurs it the same way; finally composites the blurred
// buffers over the source into the destination within the stencil, and
// swaps the buffers. Above 1920x1080 the pass starts one downsample level
// lower. Split barriers start the destination's and the quarter-size
// buffers' transitions early.
void RndPostProcStageBloomCom::_DrawImpl(
    RndContext& context,
    const RndSceneInternalContext::CameraData& camera,
    const RndSceneDrawParams& params,
    RndSceneBatchContext& batch) {
    static_cast<void>(camera);
    static_cast<void>(params);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Bloom");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());
    const bool identity = context.mUsingIdentityViewProjection;
    context.SetUsingIdentityViewProjection(true);

    RndBufferCollection& buffers = *batch.mBuffers;
    RndShaderMgr& shaders = TheRndDevice()->mShaderMgr;
    const RndResourceState readState = _SceneReadState();

    RndContext::RenderTargetParams targets;
    targets.mTargets.resize(1);
    // Not 1: the targets are not cleared. The value is not modelled.
    targets.mTargets[0].mClearMode = 2;

    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;

    const bool aboveHD = buffers.mSize.x > 1920 || buffers.mSize.y > 1080;
    const unsigned long level = aboveHD ? 1 : 0;
    RndTextureBase* half[2] = {buffers.mDownsample[level][0], buffers.mDownsample[level][1]};
    RndTextureBase* quarter[2] = {
        buffers.mDownsample[level + 1][0], buffers.mDownsample[level + 1][1]};
    RndTextureBase* const quarterOnly[2] = {buffers.mDownsample[1][0], buffers.mDownsample[1][1]};

    RndTextureBase* const source = _GetLightAccum(buffers, batch.mTarget.mSrcLightAccum);
    RndTextureBase* const dest = _GetLightAccum(buffers, batch.mTarget.mDstLightAccum);

    FixedVector<RndResourceBarrier, 6> barriers;
    RndTextureBase* const* first = mUseHalfSizeBuffer ? half : quarterOnly;
    barriers.push_back(Transition(
        RndResourceBarrierPhase::kImmediate,
        first[0],
        RndResourceState::kPixelShaderResource,
        RndResourceState::kRenderTarget));
    barriers.push_back(Transition(
        RndResourceBarrierPhase::kBegin,
        first[1],
        RndResourceState::kPixelShaderResource,
        RndResourceState::kRenderTarget));
    barriers.push_back(Transition(
        RndResourceBarrierPhase::kBegin, dest, readState, RndResourceState::kRenderTarget));
    barriers.push_back(Transition(
        RndResourceBarrierPhase::kImmediate, source, RndResourceState::kRenderTarget, readState));

    RndShaderDownsample::Params downsample;
    downsample.mSource = nullptr;
    RndShaderBlur::Params blur;
    RndTextureBase* blurred;
    if (mUseHalfSizeBuffer) {
        barriers.push_back(Transition(
            RndResourceBarrierPhase::kBegin,
            quarter[0],
            RndResourceState::kPixelShaderResource,
            RndResourceState::kRenderTarget));
        barriers.push_back(Transition(
            RndResourceBarrierPhase::kBegin,
            quarter[1],
            RndResourceState::kPixelShaderResource,
            RndResourceState::kRenderTarget));
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        targets.mTargets[0].mTexture = half[0];
        context.SetRenderTargets(targets);
        downsample.mDownsampleType = aboveHD ? kDownsampleBloom4x : kDownsampleBloom2x;
        downsample.mValueBasedBloom = mValueBased;
        downsample.mSource = source;
        shaders.mDownsampleShader->Select(context, downsample);
        RndDrawUtl::DrawQuad2D(context, quad);

        // Blur the half-size buffer across and back.
        barriers.resize(2);
        barriers[0].mBefore = RndResourceState::kRenderTarget;
        barriers[0].mAfter = RndResourceState::kPixelShaderResource;
        barriers[1].mPhase = RndResourceBarrierPhase::kEnd;
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = half[1];
        context.SetRenderTargets(targets);
        blur = RndShaderBlur::Params{0, 0, half[0], nullptr, nullptr, -1, 0, 0.0F};
        blur.mRadius = HorizontalRadius(*half[0], kHalfSizeBlurRadius);
        shaders.mBlurShader->Select(context, blur);
        RndDrawUtl::DrawQuad2D(context, quad);

        barriers[0].mResource = half[1];
        barriers[1].mPhase = RndResourceBarrierPhase::kImmediate;
        barriers[1].mResource = half[0];
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = half[0];
        context.SetRenderTargets(targets);
        blur.mDirection = 1;
        blur.mSource = half[1];
        blur.mRadius = kHalfSizeBlurRadius;
        shaders.mBlurShader->Select(context, blur);
        RndDrawUtl::DrawQuad2D(context, quad);

        // Downsample into the quarter-size buffer and blur it.
        barriers[0].mResource = half[0];
        barriers[1].mPhase = RndResourceBarrierPhase::kEnd;
        barriers[1].mResource = quarter[0];
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = quarter[0];
        context.SetRenderTargets(targets);
        downsample.mDownsampleType = kDownsampleColor2x;
        downsample.mSource = half[0];
        shaders.mDownsampleShader->Select(context, downsample);
        RndDrawUtl::DrawQuad2D(context, quad);

        barriers[0].mResource = quarter[0];
        barriers[1].mResource = quarter[1];
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = quarter[1];
        context.SetRenderTargets(targets);
        blur.mDirection = 0;
        blur.mSource = quarter[0];
        blur.mRadius = HorizontalRadius(*quarter[0], kQuarterSizeBlurRadius);
        shaders.mBlurShader->Select(context, blur);
        RndDrawUtl::DrawQuad2D(context, quad);

        barriers[0].mResource = quarter[1];
        barriers[1].mPhase = RndResourceBarrierPhase::kImmediate;
        barriers[1].mResource = quarter[0];
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = quarter[0];
        context.SetRenderTargets(targets);
        blur.mDirection = 1;
        blur.mSource = quarter[1];
        blur.mRadius = kQuarterSizeBlurRadius;
        blurred = quarter[0];
    } else {
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        targets.mTargets[0].mTexture = quarterOnly[0];
        context.SetRenderTargets(targets);
        downsample.mDownsampleType = kDownsampleBloom4x;
        downsample.mValueBasedBloom = mValueBased;
        downsample.mSource = source;
        shaders.mDownsampleShader->Select(context, downsample);
        RndDrawUtl::DrawQuad2D(context, quad);

        barriers.resize(2);
        barriers[0].mBefore = RndResourceState::kRenderTarget;
        barriers[0].mAfter = RndResourceState::kPixelShaderResource;
        barriers[1].mPhase = RndResourceBarrierPhase::kEnd;
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = quarterOnly[1];
        context.SetRenderTargets(targets);
        blur = RndShaderBlur::Params{0, 0, quarterOnly[0], nullptr, nullptr, -1, 0, 0.0F};
        blur.mRadius = HorizontalRadius(*quarterOnly[0], kQuarterSizeOnlyBlurRadius);
        shaders.mBlurShader->Select(context, blur);
        RndDrawUtl::DrawQuad2D(context, quad);

        barriers[0].mResource = quarterOnly[1];
        barriers[1].mPhase = RndResourceBarrierPhase::kImmediate;
        barriers[1].mResource = quarterOnly[0];
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
        DeselectPixelTextures(context);
        targets.mTargets[0].mTexture = quarterOnly[0];
        context.SetRenderTargets(targets);
        blur.mDirection = 1;
        blur.mSource = quarterOnly[1];
        blur.mRadius = kQuarterSizeOnlyBlurRadius;
        blurred = quarterOnly[0];
    }
    shaders.mBlurShader->Select(context, blur);
    RndDrawUtl::DrawQuad2D(context, quad);

    // Composite over the source into the destination.
    barriers[0].mResource = blurred;
    barriers[1].mPhase = RndResourceBarrierPhase::kEnd;
    barriers[1].mResource = dest;
    barriers[1].mBefore = readState;
    barriers[1].mAfter = RndResourceState::kRenderTarget;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    DeselectPixelTextures(context);
    targets.mTargets[0].mTexture = dest;
    targets.mDepthTexture =
        buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval].mDepthStencil;
    context.SetRenderTargets(targets);

    RndShaderBloom::Params bloom;
    bloom.mSource = source;
    bloom.mHalfSizeBloom = mUseHalfSizeBuffer ? half[0] : nullptr;
    bloom.mQuarterSizeBloom = blurred;
    bloom.mBloom[0] = mIntensity;
    bloom.mBloom[1] = mPower;
    bloom.mBloom[2] = mShape;
    bloom.mHuePreservation = mOverbrightHuePreservation;
    bloom.mOverbright[0] = mOverbrightWhiteningStrength;
    bloom.mOverbright[1] = mMaxOverbrightWhitening;
    shaders.mBloomShader->Select(context, bloom);
    context._SetStencilModeImpl(kCompositeStencilMode, 0, kCompositeStencilReadMask, 0);
    RndDrawUtl::DrawQuad2D(context, quad);
    context._SetStencilModeImpl(0, 0, 0, 0);

    _SwapLightAccum(batch.mTarget);
    context.SetUsingIdentityViewProjection(identity);
}

// Reconstructed from eboot.elf at 0x631430.
void RndPostProcStageBloomCom::_UpdateDrawTarget(RndSceneDrawTarget& target) {
    _SwapLightAccum(target);
}

// Reconstructed from eboot.elf at 0x631450.
Symbol RndPostProcStageBloomCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x631460.
Symbol RndPostProcStageBloomCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x631470.
int RndPostProcStageBloomCom::CurrentRev() const {
    return const_cast<RndPostProcStageBloomCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x631490.
bool RndPostProcStageBloomCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6314C0.
Component* RndPostProcStageBloomCom::AsComponent() {
    return this;
}

/// Reconstructed from eboot.elf at 0x6314D0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndPostProcStageBloomCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndPostProcStageBloomCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndPostProcStageBloomCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x631610.
PropRegistry& RndPostProcStageBloomCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x631620.
ComMetaData& RndPostProcStageBloomCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4059F0. The binary emits the factory
// with the class's Init (0x3F9720); the placement factory at 0x405A20
// constructs in given storage.
Component* RndPostProcStageBloomCom::_Create() {
    return new RndPostProcStageBloomCom();
}
