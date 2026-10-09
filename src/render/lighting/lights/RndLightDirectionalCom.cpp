// render/RndLightDirectionalCom.o (0x470600 to 0x47730F). The object also
// emits the std::function handlers its registry binds (0x475950-0x475A2B)
// and the copy of the runtime data for _Imprint (0x475A30); they are not
// modelled.
#include "render/lighting/lights/RndLightDirectionalCom.h"

#include <cmath>
#include <cstring>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransCom.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraCom.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/deferred/RndLightDirectionalDeferredShader.h"
#include "render/lighting/lights/RndLightProbeCom.h"
#include "render/lighting/shadows/RndShaderLightDirectionalShadowGen.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/shaders/RndShader.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture2DResource.h"

namespace {

// The shared header's null id and thread-group widths, at 0x1A87628, which
// the static initializer (0x477240) sets; nothing in the object reads them.
// Names not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFu};
[[maybe_unused]] int gLightDirectionalGroupSize2D = 8;  // 0x1A8762C
[[maybe_unused]] int gLightDirectionalGroupSize3D = 4;  // 0x1A87630

// Divisors and lengths are kept this far from zero.
constexpr float kEpsilon = 0.0001F;

// The 16-byte constant at the shader's offset in the buffer.
float* Constant(RndShaderCBuffer* cbuffer, unsigned long offset) {
    return reinterpret_cast<float*>(static_cast<char*>(cbuffer->mData) + offset * 16);
}

// The transform as the three rows of a 3x4 matrix, as the shaders read it.
void StoreRows(const Transform& xfm, float* rows) {
    const Hmx::Matrix3& m = xfm.m;
    const float values[12] = {
        m.x.x, m.y.x, m.z.x, xfm.v.x,
        m.x.y, m.y.y, m.z.y, xfm.v.y,
        m.x.z, m.y.z, m.z.z, xfm.v.z,
    };
    std::memcpy(rows, values, sizeof(values));
}

// The resource at the path among those inlined into the entity's layers,
// or null. Inlined into _OnResourcesLoaded. Name not in the reference map.
Resource* FindInlinedResource(const Entity* entity, ResourcePath path) {
    const EntityResource* resource = entity->mResource;
    for (const EntityResource::LayerInfo& layer : resource->mLayers) {
        for (Resource* inlined : layer.mInlineResources) {
            if (inlined->mPath.mPath == path.mPath) {
                return inlined;
            }
        }
    }
    return nullptr;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x477240).
Symbol RndLightDirectionalCom::sId("LightDirectional");
Symbol RndLightDirectionalCom::sClassName("LightDirectional");
PropRegistry RndLightDirectionalCom::sPropRegistry;
ComMetaData RndLightDirectionalCom::sMetaData;

// Reconstructed from eboot.elf at 0x470600. The cascades start as identity
// transforms (RuntimeData's constructor).
RndLightDirectionalCom::RndLightDirectionalCom()
    : mCookieTileSize{10.0F, 10.0F},
      mCastsShadows(false),
      mMaxDistance(100.0F),
      mNumCascades(1),
      mMaxOffscreenOccluderDist(50.0F),
      mSoften(true),
      mMinSoftness(0.05F),
      mMaxSoftness(1.0F),
      mMaxSoftnessDist(5.0F),
      mOnlyFlaggedObjects(true),
      mCastContext(3),
      mShadowOffset(0.05F) {}

// Reconstructed from eboot.elf at 0x475A30.
RndLightDirectionalCom::RndLightDirectionalCom(const RndLightDirectionalCom& other)
    : RndLightCom(other),
      mCookie(other.mCookie),
      mCookieTileSize{other.mCookieTileSize[0], other.mCookieTileSize[1]},
      mCastsShadows(other.mCastsShadows),
      mMaxDistance(other.mMaxDistance),
      mNumCascades(other.mNumCascades),
      mMaxOffscreenOccluderDist(other.mMaxOffscreenOccluderDist),
      mSoften(other.mSoften),
      mMinSoftness(other.mMinSoftness),
      mMaxSoftness(other.mMaxSoftness),
      mMaxSoftnessDist(other.mMaxSoftnessDist),
      mOnlyFlaggedObjects(other.mOnlyFlaggedObjects),
      mCastContext(other.mCastContext),
      mShadowOffset(other.mShadowOffset),
      mRuntimeData() {
    mDistanceFracs._Copy(other.mDistanceFracs);
    mQualitySettings._Copy(other.mQualitySettings);
}

// Reconstructed from eboot.elf at 0x470810.
RndLightDirectionalCom::~RndLightDirectionalCom() {
    RndShaderCBuffer::SafeDelete(mRuntimeData.mDeferredCBuffer);
    RndShaderCBuffer::SafeDelete(mRuntimeData.mShadowGenCBuffer);
}

// Reconstructed from eboot.elf at 0x473920.
void RndLightDirectionalCom::_PostCreate() {
    mDistanceFracs.Resize(4);
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x473960. A cookie inlined into the
// entity's resource is used before one loaded from its path; a changed
// cookie unlinks the old texture.
bool RndLightDirectionalCom::_OnResourcesLoaded() {
    RndLightCom::_OnResourcesLoaded();
    if (mRuntimeData.mDeferredCBuffer == nullptr) {
        mRuntimeData.mDeferredCBuffer =
            RndShaderCBuffer::New(*gRndDevice->mLighting.mDirectionalShader->mCBufferConfig, 0);
    }
    const ResourcePtr<RndTexture2DResource> previous(mRuntimeData.mCookieTexture.Get());
    if (mCookie == ResourcePath()) {
        mRuntimeData.mCookieTexture = nullptr;
    } else if (Resource* inlined = FindInlinedResource(mObject->mEntity, mCookie)) {
        mRuntimeData.mCookieTexture = ResourcePtr<RndTexture2DResource>(static_cast<RndTexture2DResource*>(inlined));
    } else {
        mRuntimeData.mCookieTexture = Resource::GetOrLoad<RndTexture2DResource>(mCookie, false);
    }
    if (previous.Get() != mRuntimeData.mCookieTexture.Get()) {
        if (previous.Get() != nullptr && !previous->Fail()) {
            previous->mTexture->SetLinkedTexture(nullptr, -1);
        }
        _CookieTextureChanged();
    }
    _SyncShadowResources();
    return true;
}

// Reconstructed from eboot.elf at 0x473D30.
void RndLightDirectionalCom::_Poll() {
    RndLightCom::_Poll();
}

// Reconstructed from eboot.elf at 0x473D40.
int RndLightDirectionalCom::_GetCapabilitiesImpl() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x473D50.
Symbol RndLightDirectionalCom::_GetIntensityUnitsImpl() const {
    return Symbol("W/m^2");
}

// Reconstructed from eboot.elf at 0x473DA0. Tiny falloff ranges are pushed
// out to the epsilon, keeping their sign.
void RndLightDirectionalCom::_PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) {
    RndComputeBufferStructs::RndCSLightDirectional data = {};
    _GetComputeShaderData(ctx.mCameras[0], data);
    const RndLightDirectionalDeferredShader* shader = gRndDevice->mLighting.mDirectionalShader;
    RndShaderCBuffer* cbuffer = mRuntimeData.mDeferredCBuffer;
    std::memcpy(Constant(cbuffer, shader->mLightColor), data.mColor, sizeof(data.mColor));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mLightWrapParams), data.mLightWrapParams, sizeof(data.mLightWrapParams));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mCookieScale), data.mCookieScale, sizeof(data.mCookieScale));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mLightDirCamSpace), data.mDirection, sizeof(data.mDirection));
    cbuffer->mSyncPending = true;
    std::memcpy(Constant(cbuffer, shader->mCamToLightXfm), data.mCamToLightXfm, sizeof(data.mCamToLightXfm));
    cbuffer->mSyncPending = true;

    if (const RndLightProbeCom* light = probe.mProbe) {
        *Constant(cbuffer, shader->mProbeBlendAmount) = probe.mStateBlend;
        float range = light->mFalloffEnd - light->mFalloffStart;
        if (range < 0.0F ? range > -kEpsilon : range < kEpsilon) {
            range = range < 0.0F ? -kEpsilon : kEpsilon;
        }
        const float scale = 1.0F / range;
        float* falloff = Constant(cbuffer, shader->mProbeFalloffParams);
        falloff[0] = scale;
        falloff[1] = -(light->mFalloffStart * scale);
        falloff[2] = static_cast<float>(light->mFalloffFunction);
        cbuffer->mSyncPending = true;

        Transform worldToProbe;
        Invert(light->mRuntime.mTrans->mWorldXfm, worldToProbe);
        Transform camToProbe;
        Multiply(ctx.mCameras[0].mViews[0].mWorldXfm, worldToProbe, camToProbe);
        StoreRows(camToProbe, Constant(cbuffer, shader->mCamToProbeXfm));
        cbuffer->mSyncPending = true;
    }
    if (cbuffer->mSyncPending) {
        cbuffer->_SyncImpl(ctx, 0, cbuffer->mNumElements);
        cbuffer->mSyncPending = false;
    }
}

// Reconstructed from eboot.elf at 0x4741D0.
void RndLightDirectionalCom::_GetComputeShaderData(
    const RndCameraContext& camera,
    RndComputeBufferStructs::RndCSLightDirectional& data) const {
    data.mVolumetric = mVolumetric;
    data.mIlluminationType = mIlluminationType;
    data.mEnvironBits = mRuntime.mEnvironBits;
    data.mCookieIndex = mRuntime.mTiledCookieIndex != static_cast<unsigned long>(-1)
        ? (mRuntime.mCookieTexArrayIndex << 11) + static_cast<int>(mRuntime.mTiledCookieIndex)
        : -1;

    const float intensity = mRuntime.mMasterIntensityMult * mIntensity;
    if (mIlluminationType == 3) {
        const float blend = intensity < 1.0F ? intensity : 1.0F;
        const Hmx::Color& white = Hmx::Color::GetWhite();
        data.mColor[0] = (mColor.red - white.red) * blend + white.red;
        data.mColor[1] = (mColor.green - white.green) * blend + white.green;
        data.mColor[2] = (mColor.blue - white.blue) * blend + white.blue;
    } else {
        data.mColor[0] = intensity * mColor.red;
        data.mColor[1] = intensity * mColor.green;
        data.mColor[2] = intensity * mColor.blue;
    }

    if (mLightWrap > 2.0F) {
        const float wrap = mLightWrap * 0.25F;
        const float diffuse = 1.0F - wrap;
        data.mLightWrapParams[0] = diffuse > 0.0F ? diffuse : 0.0F;
        data.mLightWrapParams[1] = wrap;
    } else {
        const float scale = 1.0F / (mLightWrap > kEpsilon ? mLightWrap : kEpsilon);
        data.mLightWrapParams[0] = scale;
        data.mLightWrapParams[1] = 1.0F - scale;
    }

    // The light travels along the object's -y axis.
    const Vector3& axis = mRuntime.mTrans->mWorldXfm.m.y;
    const RndCameraContext::View& view = camera.mViews[0];
    const Hmx::Matrix3& rotation = view.mWorldRotationTranspose;
    const float x = -(axis.x * rotation.x.x + axis.y * rotation.y.x + axis.z * rotation.z.x);
    const float y = -(axis.x * rotation.x.y + axis.y * rotation.y.y + axis.z * rotation.z.y);
    const float z = -(axis.x * rotation.x.z + axis.y * rotation.y.z + axis.z * rotation.z.z);
    const float length = std::sqrt(x * x + y * y + z * z);
    const float scale = length != 0.0F ? 1.0F / length : 0.0F;
    data.mDirection[0] = scale * x;
    data.mDirection[1] = scale * y;
    data.mDirection[2] = scale * z;

    Transform camToLight;
    Multiply(view.mWorldXfm, mRuntime.mDrawNode->mRuntime.mInvWorldXfm, camToLight);
    StoreRows(camToLight, &data.mCamToLightXfm[0][0]);

    data.mCookieScale[0] = 1.0F / (mCookieTileSize[0] > kEpsilon ? mCookieTileSize[0] : kEpsilon);
    data.mCookieScale[1] = -1.0F / (mCookieTileSize[1] > kEpsilon ? mCookieTileSize[1] : kEpsilon);
}

// Reconstructed from eboot.elf at 0x474530. The probe's second-state
// textures make the shader combine with the probe; the blend the probe
// returns is not used.
void RndLightDirectionalCom::_DrawDeferredNoComputeImpl(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    const RndLightProbeParams& probe) {
    static_cast<void>(buffers);
    static_cast<void>(target);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Directional Light");
    }
    static Symbol sProbeStatName;
    if (sProbeStatName == Symbol()) {
        sProbeStatName = Symbol("Directional Light + Probe");
    }
    RndScopedGpuStatBlock statBlock(ctx, probe.mProbe != nullptr ? sProbeStatName.Str() : sStatName.Str());
    mRuntimeData.mDeferredCBuffer->_SelectImpl(ctx);

    RndLightDirectionalDeferredShader::Params params;
    params.mShadowMap = nullptr;
    params.mCookie = nullptr;
    params.mCombineWithProbe = false;
    params.mCombineWithProbeBlend = false;
    std::memset(params.mProbeTextures, 0, sizeof(params.mProbeTextures));
    params.mIllumType = mIlluminationType;
    params.mCookie = RndLightDirectionalCom::_GetCookieTextureImpl();
    if (probe.mProbe != nullptr) {
        float blend;
        // The shader's slots hold the cube textures as RndTextureBase.
        if (probe.mProbe->_GetBlendTextures(
                probe.mStateIndices[0],
                probe.mStateIndices[1],
                reinterpret_cast<RndTextureCube**>(params.mProbeTextures[0]),
                reinterpret_cast<RndTextureCube**>(params.mProbeTextures[1]),
                &blend,
                probe.mStateBlend)) {
            params.mCombineWithProbe = true;
        }
    }
    gRndDevice->mLighting.mDirectionalShader->Select(ctx, params);
    ctx._SetDepthModeImpl(0);
    ctx._SetCullModeImpl(kCullNone);

    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    quad.mKeepState = true;
    RndDrawUtl::DrawQuad2D(ctx, quad);
}

// Reconstructed from eboot.elf at 0x4747E0. The light's data is appended to
// the buffer's staging copy.
void RndLightDirectionalCom::_AddToComputeBufferImpl(
    RndQualityLevel quality,
    const RndCameraContext& camera,
    RndComputeBuffer& buffer) {
    static_cast<void>(quality);
    RndComputeBufferStructs::RndCSLightDirectional data = {};
    _GetComputeShaderData(camera, data);
    std::memcpy(static_cast<char*>(buffer.mStagingData) + buffer.mStagingSize, &data, sizeof(data));
    buffer.mStagingSize += sizeof(data);
}

// Reconstructed from eboot.elf at 0x4748C0.
RndTextureBase* RndLightDirectionalCom::_GetCookieTextureImpl() const {
    RndTexture2DResource* cookie = mRuntimeData.mCookieTexture.Get();
    if (cookie == nullptr || cookie->Fail()) {
        return nullptr;
    }
    return cookie->mTexture;
}

// Reconstructed from eboot.elf at 0x474900.
bool RndLightDirectionalCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    if (!mCastsShadows) {
        return false;
    }
    return mQualitySettings[static_cast<int>(quality)].mCastsShadows;
}

// Reconstructed from eboot.elf at 0x474EE0. Bit 3 of the TransCom's flags
// marks a world transform set from outside.
void RndLightDirectionalCom::_SetCascadeCamera(unsigned int cascade, const Frustum& frustum) {
    const ShadowCascade camera = _CalcCascadeCamera(frustum);
    RndCameraCom* shadowCamera = mRuntimeData.mShadowCamera;
    shadowCamera->mOrthographic = true;
    shadowCamera->mNearDistance = camera.mNearPlane;
    shadowCamera->mFarDistance = camera.mFarPlane;
    if (!shadowCamera->mPixelAccurate) {
        shadowCamera->mOrthoHeight = camera.mOrthoHeight;
    }
    TransCom* trans = shadowCamera->mObject->GetExistingCom<TransCom>();
    trans->mWorldXfm = camera.mXfm;
    trans->mDirtyFlags |= 8;
    mRuntimeData.mCascades[cascade] = camera;
}

// Reconstructed from eboot.elf at 0x474FE0.
bool RndLightDirectionalCom::_AcquireDeferredShadowContributionResourcesImpl(
    bool castsShadows,
    RndLightMgrCom& mgr) {
    mRuntimeData.mShadowMapLayer = -1;
    mRuntimeData.mShadowContributionIndex = -1;
    if (!castsShadows) {
        return false;
    }
    mRuntimeData.mShadowMapLayer = mgr.AcquireSpotShadowDepthLayers(mNumCascades);
    if (mRuntimeData.mShadowMapLayer == -1) {
        return false;
    }
    mRuntimeData.mShadowContributionIndex = mgr.AcquireShadowContribution();
    return mRuntimeData.mShadowContributionIndex != -1 && mRuntimeData.mShadowMapLayer != -1;
}

// Reconstructed from eboot.elf at 0x475730.
Symbol RndLightDirectionalCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x475740.
Symbol RndLightDirectionalCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x475750.
int RndLightDirectionalCom::CurrentRev() const {
    return const_cast<RndLightDirectionalCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x475770.
bool RndLightDirectionalCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4757A0.
Component* RndLightDirectionalCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4757B0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightDirectionalCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightDirectionalCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightDirectionalCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4758C0.
PropRegistry& RndLightDirectionalCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4758D0.
ComMetaData& RndLightDirectionalCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4758E0.
int RndLightDirectionalCom::_GetTypeImpl() const {
    return 2;
}
