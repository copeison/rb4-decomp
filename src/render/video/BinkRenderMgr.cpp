#include "render/video/BinkRenderMgr.h"

#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "render/video/RndShaderBinkConvert.h"
#include "utl/containers/FixedVector.h"
#include "utl/text/Symbol.h"

BinkRenderMgr* BinkRenderMgr::sInstance = nullptr;

// Reconstructed from eboot.elf at 0x5F2B40. The conversions draw without a
// camera.
void BinkRenderMgr::PrepareFrame(RndContext& context) {
    context.SetCamera(nullptr);
    for (auto* video : mConversions) {
        if (video != nullptr && video->mConversionPending) {
            ConvertFrame(context, *video);
            video->mConversionPending = false;
        }
    }
}

// Reconstructed from eboot.elf at 0x5F2BF0. The output leaves the pixel-
// shader-resource state for the draw and returns to it afterwards. The
// planes are drawn into it with a full-target quad; the full scale stays
// zero.
void BinkRenderMgr::ConvertFrame(RndContext& context, BinkRenderVideo& video) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Bink Convert");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    FixedVector<RndResourceBarrier, 1> barriers;
    RndResourceBarrier barrier;
    barrier.mResource = video.mOutput;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = RndResourceState::kPixelShaderResource;
    barrier.mAfter = RndResourceState::kRenderTarget;
    barriers.push_back(barrier);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndContext::RenderTargetParams targets;
    // Clear mode 2 keeps the target's contents.
    targets.mTargets.push_back({video.mOutput, 2, -1});
    context.SetRenderTargets(targets);

    RndShaderBinkConvert::Params params = {};
    params.mYPlane = video.mYPlane;
    params.mCRPlane = video.mCRPlane;
    params.mCBPlane = video.mCBPlane;
    params.mAPlane = video.mAPlane;
    for (int i = 0; i < 4; ++i) {
        params.mYScale[i] = video.mYScale[i];
        params.mCRScale[i] = video.mCRScale[i];
        params.mCBScale[i] = video.mCBScale[i];
        params.mFullOffset[i] = video.mOffset[i];
    }
    TheRndDevice()->mShaderMgr.mBinkConvertShader->Select(context, params);

    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(context, quad);

    barriers[0].mBefore = RndResourceState::kRenderTarget;
    barriers[0].mAfter = RndResourceState::kPixelShaderResource;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
}
