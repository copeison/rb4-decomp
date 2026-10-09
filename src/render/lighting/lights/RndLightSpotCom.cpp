// render/RndLightSpotCom.o (0x49F2E0-0x4A867F). The PropArray element
// operations of "quality_settings" (vtable 0x19079A0), the element type's
// registration (0x4A2120), the registry's std::function handlers
// (0x4A6FD0 on) and the static initializer are not reconstructed.
#include "render/lighting/lights/RndLightSpotCom.h"

#include <cmath>
#include <new>

#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/scalar/Trig.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector4.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/lighting/RndLightGlobals.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture2DResource.h"

namespace {

constexpr float kPi = 3.1415927F;
constexpr float kHalfPi = 1.5707964F;

// The square root the binary computes with a reciprocal square root and
// one Newton step, zero for zero.
float Length(float lengthSquared) {
    return lengthSquared == 0.0F ? 0.0F : std::sqrt(lengthSquared);
}

}  // namespace

Symbol RndLightSpotCom::sId("LightSpot");
Symbol RndLightSpotCom::sClassName("LightSpot");
PropRegistry RndLightSpotCom::sPropRegistry;
ComMetaData RndLightSpotCom::sMetaData;

// Reconstructed from eboot.elf at 0x49F2E0. The RuntimeData constructor is
// inlined here.
RndLightSpotCom::RndLightSpotCom()
    : mBulbRadius(0.05F),
      mFalloffStart(0.0F),
      mFalloffEnd(20.0F),
      mFalloffFunction(0),
      mStartAngle(0.0F),
      mEndAngle(0.7853982F),
      mAngleFalloffFunction(0),
      mTruncation(0.0F),
      mCastsShadows(false),
      mOnlyFlaggedObjects(true),
      mCastContext(3),
      mShadowOffset(0.05F),
      mShadowSoftnessMin(0.1F),
      mShadowSoftnessMax(1.0F),
      mMaxShadowSoftnessDistance(5.0F),
      mCookieTiling{1.0F, 1.0F} {}

// Reconstructed from eboot.elf at 0x49F4A0.
RndLightSpotCom::RuntimeData::RuntimeData()
    : mGeometryDirty(false),
      mDeferredCBuffer(nullptr),
      mShadowGenCBuffer(nullptr),
      mWorldCapSphere(),
      mCameraInside(false),
      mUseGeoClipPlane(false),
      mShadowMapIndex(-1),
      mShadowContributionIndex(-1),
      mShadowCameraId{0xFFFFFFFFU},
      mShadowCamera(nullptr) {
    mGeoClipPlane.c = 0.0F;
}

// Reconstructed from eboot.elf at 0x49F550.
RndLightSpotCom::~RndLightSpotCom() {
    RndShaderCBuffer::SafeDelete(mRuntimeData.mDeferredCBuffer);
    RndShaderCBuffer::SafeDelete(mRuntimeData.mShadowGenCBuffer);
}

// Reconstructed from eboot.elf at 0x49F690.
void RndLightSpotCom::SetFalloffStart(float distance) {
    mFalloffStart = distance;
    const float end = distance > mFalloffEnd ? distance : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F6E0.
void RndLightSpotCom::_SyncFalloffStart() {
    const float end = mFalloffEnd > mFalloffStart ? mFalloffEnd : mFalloffStart;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F720.
void RndLightSpotCom::SetFalloffEnd(float distance) {
    mFalloffEnd = distance;
    mFalloffStart = distance < mFalloffStart ? distance : mFalloffStart;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F760.
void RndLightSpotCom::_SyncFalloffEnd() {
    mFalloffStart = mFalloffEnd < mFalloffStart ? mFalloffEnd : mFalloffStart;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F7A0.
void RndLightSpotCom::SetStartAngle(float angle) {
    mStartAngle = angle;
    const float end = angle > mEndAngle ? angle : mEndAngle;
    const bool changed = end != mEndAngle;
    mEndAngle = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F7F0.
void RndLightSpotCom::_SyncStartAngle() {
    const float end = mEndAngle > mStartAngle ? mEndAngle : mStartAngle;
    const bool changed = end != mEndAngle;
    mEndAngle = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F830.
void RndLightSpotCom::SetEndAngle(float angle) {
    mEndAngle = angle;
    mStartAngle = angle < mStartAngle ? angle : mStartAngle;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F870.
void RndLightSpotCom::_SyncEndAngle() {
    mStartAngle = mEndAngle < mStartAngle ? mEndAngle : mStartAngle;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F8B0. A cosine range closer to zero
// than 0.0001 is pushed out to it.
Vector3 RndLightSpotCom::GetAngleFalloffParams(float scale) const {
    const float outerAngle = kHalfPi < scale * mRuntimeData.mCone.mAngle ? kHalfPi : scale * mRuntimeData.mCone.mAngle;
    const float innerAngle = kHalfPi < scale * 0.5F * mStartAngle ? kHalfPi : scale * 0.5F * mStartAngle;
    const float innerCosine = Sine(innerAngle + kHalfPi);
    const float outerCosine = Sine(kHalfPi + outerAngle);
    int function;
    if (mAngleFalloffFunction != 0) {
        function = mAngleFalloffFunction - 1;
    } else {
        function = mIlluminationType == 3 ? 3 : 1;
    }
    float range = outerCosine - innerCosine;
    const bool tooSmall = range < 0.0F ? range > -0.0001F : range < 0.0001F;
    if (tooSmall) {
        range = range < 0.0F ? -0.0001F : 0.0001F;
    }
    const float reciprocal = 1.0F / range;
    return Vector3{reciprocal, -(innerCosine * reciprocal), static_cast<float>(function)};
}

// Reconstructed from eboot.elf at 0x4A22D0.
void RndLightSpotCom::_PostCreate() {
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x4A22F0.
void RndLightSpotCom::_PreDestroy(DestroyType type) {
    RndLightCom::_PreDestroy(type);
    if (type != kDestroyComponent) {
        return;
    }
    RndDrawNodeCom* node = mObject->GetCom<RndDrawNodeCom>();
    if (node == nullptr) {
        return;
    }
    const Transform& xfm = mObject->GetExistingCom<TransCom>()->mWorldXfm;
    node->SetLocalSphere(Sphere{xfm.v, 0.0F});
}

// Reconstructed from eboot.elf at 0x4A28A0. The light globals' spotlight
// mesh has mSpotlightSegments sides around the axis and mSpotlightCapSegments
// rings on the cap; the mesh cone grows by the reciprocal cosines of their
// half steps so the flat facets stay outside the volume.
void RndLightSpotCom::_SyncGeometry() {
    const float angle = mEndAngle * 0.5F;
    float topRadius = 0.0F;
    if (angle > 0.0F) {
        const float cosine = Sine(angle + kHalfPi);
        topRadius = mFalloffEnd * Sine(angle) / (1.0F - cosine);
    }
    TruncatedRoundedCone& cone = mRuntimeData.mCone;
    cone.SetAngleTopRadiusAndLength(angle, topRadius * mTruncation, mFalloffEnd);

    const RndLightGlobals& lighting = TheRndDevice()->mLighting;
    const float sideStep = kPi / static_cast<float>(lighting.mSpotlightSegments);
    const float capStep = cone.mAngle / static_cast<float>(lighting.mSpotlightCapSegments);
    const float sideScale = 1.0F / Sine(sideStep + kHalfPi);
    const float scale = 1.0F / Sine(kHalfPi + capStep) * sideScale;
    const float length = (cone.mLength - cone.mConeLength) * scale + cone.mConeLength;
    mRuntimeData.mMeshCone.SetRadiiAndLength(scale * cone.mTopRadius, scale * cone.mBottomRadius, length);
}

// Reconstructed from eboot.elf at 0x4A2B20. The map's _SyncLocalSphere is
// inlined: the sphere through the mesh cone's top rim and its cap rim.
void RndLightSpotCom::_Poll() {
    RndLightCom::_Poll();
    if (mRuntimeData.mGeometryDirty) {
        const TruncatedRoundedCone& meshCone = mRuntimeData.mMeshCone;
        const float bottomSquared = meshCone.mBottomRadius * meshCone.mBottomRadius;
        float centerDepth = 0.0F;
        if (0.0F < meshCone.mConeLength) {
            const float depth =
                (meshCone.mConeLength * meshCone.mConeLength + bottomSquared -
                 meshCone.mTopRadius * meshCone.mTopRadius) /
                (meshCone.mConeLength + meshCone.mConeLength);
            centerDepth = meshCone.mConeLength < depth ? meshCone.mConeLength : depth;
        }
        const float rimDepth = meshCone.mConeLength - centerDepth;
        const float radius = Length(rimDepth * rimDepth + bottomSquared);
        mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(
            Sphere{Vector3{0.0F, 0.0F, -centerDepth}, radius});
        mRuntimeData.mGeometryDirty = false;
    }
    if ((mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) == 0 && IsOn()) {
        const Transform& xfm = mRuntime.mTrans->mWorldXfm;
        const Hmx::Matrix3& m = xfm.m;
        const float xScaleSquared = m.x.x * m.x.x + m.x.y * m.x.y + m.x.z * m.x.z;
        const float yScaleSquared = m.y.x * m.y.x + m.y.y * m.y.y + m.y.z * m.y.z;
        const float sideScale = Length(yScaleSquared > xScaleSquared ? yScaleSquared : xScaleSquared);
        const float zScaleSquared = m.z.x * m.z.x + m.z.y * m.z.y + m.z.z * m.z.z;
        const float zScale = Length(zScaleSquared);
        const float zInverse = 1.0F / std::sqrt(zScaleSquared);
        const TruncatedRoundedCone& cone = mRuntimeData.mCone;
        const float length = zScale * cone.mLength;
        const Vector3 end{
            xfm.v.x - m.z.x * zInverse * length,
            xfm.v.y - m.z.y * zInverse * length,
            xfm.v.z - m.z.z * zInverse * length};
        const float endRadius = sideScale * (cone.mCapRadius / cone.mCapRimHeight * cone.mBottomRadius);
        mRuntimeData.mWorldBounds.Set(xfm.v, end, sideScale * cone.mTopRadius, endRadius);
        const float capOffset = cone.mCapRadius - cone.mLength;
        mRuntimeData.mWorldCapSphere.center.x = capOffset * m.z.x + xfm.v.x;
        mRuntimeData.mWorldCapSphere.center.y = capOffset * m.z.y + xfm.v.y;
        mRuntimeData.mWorldCapSphere.center.z = capOffset * m.z.z + xfm.v.z;
        mRuntimeData.mWorldCapSphere.radius = cone.mCapRadius;
    }
    if (mCastsShadows && (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) == 0 && IsOn()) {
        _SyncShadowPlanes();
    }
}

// Reconstructed from eboot.elf at 0x4A3050.
int RndLightSpotCom::_GetCapabilitiesImpl() const {
    return 3;
}

// Reconstructed from eboot.elf at 0x4A3060.
Symbol RndLightSpotCom::_GetIntensityUnitsImpl() const {
    return Symbol("W/sr");
}

// Reconstructed from eboot.elf at 0x4A4CD0.
bool RndLightSpotCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    if (!mCastsShadows) {
        return false;
    }
    const auto* settings = static_cast<const ShadowQualitySettings*>(mQualitySettings.mData);
    return settings[static_cast<int>(quality)].mCastsShadows;
}

// Reconstructed from eboot.elf at 0x4A52B0.
bool RndLightSpotCom::_AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) {
    if (!castsShadows) {
        mRuntimeData.mShadowMapIndex = -1;
        mRuntimeData.mShadowContributionIndex = -1;
        return false;
    }
    mRuntimeData.mShadowMapIndex = mgr.AcquireSpotShadowDepthLayers(1);
    long contribution = -1;
    if (mRuntimeData.mShadowMapIndex != -1) {
        contribution = mgr.AcquireShadowContribution();
    }
    mRuntimeData.mShadowContributionIndex = contribution;
    return contribution != -1 && mRuntimeData.mShadowMapIndex != -1;
}

// Reconstructed from eboot.elf at 0x4A6920.
bool RndLightSpotCom::_IsOnImpl() const {
    return std::fabs(mFalloffEnd) > 0.0001F && std::fabs(mEndAngle) > 0.0001F;
}

// Reconstructed from eboot.elf at 0x4A6960.
void RndLightSpotCom::_SyncTruncation() {
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x4A6980. The depth range runs from the
// cap's rim to its end, at least 0.1 and 0.2.
void RndLightSpotCom::_CalcShadowParams(Vector4& projection, Vector4& depthRange) const {
    if (!mCastsShadows) {
        return;
    }
    const TruncatedRoundedCone& cone = mRuntimeData.mCone;
    const float farDepth = cone.mCapRadius > 0.2F ? cone.mCapRadius : 0.2F;
    const float rimDepth = cone.mCapRadius - cone.mLength;
    const float nearDepth = rimDepth > 0.1F ? rimDepth : 0.1F;
    depthRange.x = nearDepth;
    depthRange.y = farDepth;
    depthRange.z = nearDepth * farDepth;
    depthRange.w = farDepth - nearDepth;
    const float bottomRadius = cone.mBottomRadius > 0.0001F ? cone.mBottomRadius : 0.0001F;
    const float slope = cone.mCapRimHeight / bottomRadius;
    projection.x = slope * -0.5F;
    projection.y = slope * 0.5F;
    projection.z = 0.5F;
    projection.w = 0.5F;
}

// Reconstructed from eboot.elf at 0x4A6C50.
Symbol RndLightSpotCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4A6C60.
Symbol RndLightSpotCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4A6C70.
int RndLightSpotCom::CurrentRev() const {
    return const_cast<RndLightSpotCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4A6C90.
bool RndLightSpotCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4A6CC0.
Component* RndLightSpotCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4A6CD0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightSpotCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightSpotCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightSpotCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4A6FA0.
PropRegistry& RndLightSpotCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4A6FB0.
ComMetaData& RndLightSpotCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4A6FC0.
int RndLightSpotCom::_GetTypeImpl() const {
    return 1;
}

// Reconstructed from eboot.elf at 0x404700. The binary emits the factory
// with the class's Init (0x3F08C0), before the renderer's components.
Component* RndLightSpotCom::_Create() {
    return new RndLightSpotCom();
}
