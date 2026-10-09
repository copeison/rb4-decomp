// render/RndFogCom.o (0x451D10 to 0x452C9F). The object also emits
// RndAtmosphereCom::_InitAsSuperclass (0x4522E0), which the volumetric
// scattering shares.
#include "render/atmosphere/RndFogCom.h"

#include <cstring>
#include <new>

#include "entity/props/PropArray.h"
#include "math/color/Color.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/lighting/fog/RndShaderFogDeferred.h"
#include "render/materials/RndMaterialCom.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTexture2D.h"

// The object's statics, in the order of its static initializer (0x452BD0).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndFogCom::sId("Fog");
Symbol RndFogCom::sClassName("Fog");
PropRegistry RndFogCom::sPropRegistry;
ComMetaData RndFogCom::sMetaData;

namespace {

// The default falloff function, and the scale of a fog with no range.
constexpr int kDefaultFalloffFunction = 2;
constexpr float kNoRangeScale = 10000.0F;

// The light accumulation buffer the target selects, or the frame
// interval's own one outside a scene context. Inlined at each use.
RndTextureBase* LightAccum(
    const RndBufferCollection& buffers,
    unsigned long index) {
    if (buffers.mActiveSceneContext != 0) {
        return buffers.mLightAccum[index];
    }
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
        .mPartialLightAccum;
}

}  // namespace

// Reconstructed from eboot.elf at 0x451D10.
RndFogCom::RndFogCom()
    : mFalloffFunction(kDefaultFalloffFunction),
      mRuntime{{0.0F, 0.0F, 0.0F}, nullptr} {}

// Reconstructed from eboot.elf at 0x451D60. The deleting destructor is at
// 0x451D90.
RndFogCom::~RndFogCom() {
    RndShaderCBuffer::SafeDelete(mRuntime.mCBuffer);
}

// Reconstructed from eboot.elf at 0x452390. The fog blends over the light
// accumulation with a full-target quad, testing the stencil.
void RndFogCom::ApplyDeferred(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target,
    RndTextureBase* atmosphereTexture) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Fog");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndDevice* const device = TheRndDevice();
    const RndBufferCollection::FrameIntervalBuffers& frame =
        buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    context.SetRenderTargets(
        LightAccum(buffers, target.mSrcLightAccum), frame.mDepthStencil);
    RndShaderCBuffer* cbuffer = mRuntime.mCBuffer;
    if (cbuffer->mSyncPending) {
        cbuffer->_SyncImpl(context, 0, cbuffer->mNumElements);
        cbuffer->mSyncPending = false;
    }
    cbuffer->_SelectImpl(context);

    RndShaderFogDeferred::Params params;
    params.mBuffers = &buffers;
    params.mTarget = target;
    params.mSkyTexture = atmosphereTexture != nullptr
        ? atmosphereTexture
        : device->mDefaults.mTextures2D[kDefaultTextureBlack];
    device->mAtmosphere.mFogDeferred->Select(context, params);

    context.mBlendMode = RndBlendMode::kSourceAlpha;
    context._SetBlendModeImpl(
        RndBlendMode::kSourceAlpha, Hmx::Color::GetWhite());
    context._SetDepthModeImpl(0);
    context._SetCullModeImpl(kCullNone);
    context._SetStencilModeImpl(2, 0, 7, 0);
    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    quad.mKeepState = true;
    RndDrawUtl::DrawQuad2D(context, quad);
    context._SetStencilModeImpl(0, 0, 0, 0);
}

// Reconstructed from eboot.elf at 0x452690.
RndTextureBase* RndFogCom::TextureOrDefault(
    RndTextureBase* texture,
    unsigned int mode) const {
    RndTexture2D* const* defaults = TheRndDevice()->mDefaults.mTextures2D;
    switch (mode) {
    case 0:
    case 2:
    case 4:
    case 5:
        if (texture != nullptr) {
            return texture;
        }
        return defaults[kDefaultTextureBlack];
    case 1:
    case 3:
    case 6:
    case 7:
    case 9:
        return defaults[kDefaultTextureBlack];
    case 8:
    case 10:
        return defaults[kDefaultTextureWhite];
    default:
        return defaults[kDefaultTextureError];
    }
}

// Reconstructed from eboot.elf at 0x4526E0.
void RndFogCom::SetFwdShadingConstants(RndShaderCBuffer& cbuffer) const {
    const unsigned long offset = TheRndDevice()->mShaderMgr.mFogParams;
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, offset),
        &mRuntime.mFalloffParams,
        sizeof(mRuntime.mFalloffParams));
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x452710.
void RndFogCom::SetNoAtmosphereFwdShadingConstants(
    RndShaderCBuffer& cbuffer) {
    const unsigned long offset = TheRndDevice()->mShaderMgr.mFogParams;
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, offset),
        &Vector3::sZero,
        sizeof(Vector3::sZero));
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x452750.
bool RndFogCom::_OnResourcesLoaded() {
    if (mRuntime.mCBuffer == nullptr) {
        mRuntime.mCBuffer = RndShaderCBuffer::New(
            *TheRndDevice()->mAtmosphere.mFogDeferred->mCBufferConfig, 0);
    }
    _SyncFalloffParams();
    return true;
}

// Reconstructed from eboot.elf at 0x452820.
void RndFogCom::_SyncFalloffParams() {
    const float range = mEndDist - mStartDist;
    const float scale = range != 0.0F ? 1.0F / range : kNoRangeScale;
    mRuntime.mFalloffParams.x = scale;
    mRuntime.mFalloffParams.y = -(mStartDist * scale);
    mRuntime.mFalloffParams.z = static_cast<float>(mFalloffFunction) + 0.1F;
    const unsigned long offset =
        TheRndDevice()->mAtmosphere.mFogDeferred->mFalloffParams;
    RndShaderCBuffer& cbuffer = *mRuntime.mCBuffer;
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, offset),
        &mRuntime.mFalloffParams,
        sizeof(mRuntime.mFalloffParams));
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x4528B0.
void RndFogCom::_Poll() {
    _SyncFalloffParams();
}

// Reconstructed from eboot.elf at 0x452940.
void RndFogCom::_EditPoll() {
    _SyncFalloffParams();
}

// Reconstructed from eboot.elf at 0x4529D0.
Symbol RndFogCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4529E0.
Symbol RndFogCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4529F0.
int RndFogCom::CurrentRev() const {
    return const_cast<RndFogCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x452A10.
bool RndFogCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x452A40.
Component* RndFogCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x452A50. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndFogCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndFogCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndFogCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x452BB0.
PropRegistry& RndFogCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x452BC0.
ComMetaData& RndFogCom::_GetMetaData() {
    return sMetaData;
}
