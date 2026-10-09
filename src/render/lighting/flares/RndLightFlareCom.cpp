// render/RndLightFlareCom.o (0x47A3C0 to 0x47E7AF). The registry builder
// (_Init, 0x47A780), the subflare type's registration (the map's
// PropArray<RndLightFlareCom::Subflare>::Init(Symbol), 0x47BC70) and the
// property handlers after the identity slots (0x47D880-0x47E4D0) are not
// reconstructed. The object's static initializer is at 0x47E6B0.
#include "render/lighting/flares/RndLightFlareCom.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/geometry/Sphere.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/lighting/lights/RndLightSpotCom.h"
#include "render/materials/RndMaterialCom.h"
#include "render/materials/RndMaterialReferenceCom.h"
#include "render/materials/RndMaterialRuntimeData.h"
#include "render/options/RndLightOptionsCom.h"
#include "render/queries/RndOcclusionQuery.h"
#include "render/queries/RndOcclusionQueryMgr.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x47E6DC; see RndTypesetter.cpp). This object
// reads the -1 as the null object id. Names not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};              // 0x1A87DE8
[[maybe_unused]] int gLightFlareGroupSize2D = 8;  // 0x1A87DEC
[[maybe_unused]] int gLightFlareGroupSize3D = 4;  // 0x1A87DF0

// The smallest length and divisor the subflare math uses. Name not in the
// reference map.
constexpr float kMinDivisor = 0.0001F;
constexpr float kPi = 3.14159265F;

// The instances of a list, which the component writes in place. Name not
// in the reference map.
RndDrawInstance* Instances(const VectorAdapter<RndDrawInstance>& list) {
    return const_cast<RndDrawInstance*>(list.mData);
}

// The reciprocal of the length, or 0 for a zero vector. Name not in the
// reference map.
float InverseLength(float x, float y, float z) {
    const float length = std::sqrt(x * x + y * y + z * z);
    return length != 0.0F ? 1.0F / length : 0.0F;
}

// Rounds half away from zero, saturating. Name not in the reference map.
int RoundToInt(float value) {
    if (value > 0.0F) {
        value += 0.5F;
        return value < 2147483648.0F ? static_cast<int>(value)
                                     : std::numeric_limits<int>::max();
    }
    value -= 0.5F;
    return value > -2147483648.0F ? static_cast<int>(value)
                                  : std::numeric_limits<int>::min();
}

// The material of the object with the id: its RndMaterialCom, or the one
// its RndMaterialReferenceCom names; null for the null id. Inlined into
// slots 43 and 44. Name not in the reference map.
RndMaterialCom* FindMaterial(const Entity* entity, GameObjectId id) {
    if (id.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* object = entity->GetObject(id);
    if (object == nullptr) {
        return nullptr;
    }
    RndMaterialCom* material = object->GetCom<RndMaterialCom>();
    if (material != nullptr) {
        return material;
    }
    const auto* reference = object->GetCom<RndMaterialReferenceCom>();
    if (reference == nullptr || reference->mMaterial.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* target =
        reference->mObject->mEntity->GetObject(reference->mMaterial);
    return target != nullptr ? target->GetCom<RndMaterialCom>() : nullptr;
}

// The material of the subflares without one: the object's, the referenced
// object's, or the class's default, as RndDrawInstanceCom resolves it.
// Inlined into slots 43 and 44. Name not in the reference map.
RndMaterialCom* ObjectMaterial(const RndDrawInstanceCom::RuntimeData& data) {
    RndMaterialCom* material = data.mMaterial;
    if (material != nullptr) {
        return material;
    }
    const RndMaterialReferenceCom* reference = data.mMaterialReference;
    if (reference != nullptr && reference->mMaterial.mId != gNullObjectId.mId) {
        const GameObject* object =
            reference->mObject->mEntity->GetObject(reference->mMaterial);
        if (object != nullptr) {
            material = object->GetCom<RndMaterialCom>();
        }
    }
    return material != nullptr ? material : data.mDefaultMaterial;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x47E6B0).
Symbol RndLightFlareCom::sId("LightFlare");
Symbol RndLightFlareCom::sClassName("LightFlare");
PropRegistry RndLightFlareCom::sPropRegistry;
ComMetaData RndLightFlareCom::sMetaData;

// Reconstructed from eboot.elf at 0x47A3C0.
RndLightFlareCom::RndLightFlareCom()
    : mLightSourceRadius(0.1F),
      mSize{0.1F, 0.1F},
      mAngleRadians(0.0F),
      mLight{0xFFFFFFFFU},
      mColor(Hmx::Color::GetWhite()),
      mIntensity(1.0F),
      mCamCenteringIntensity(0.5F),
      mSpotlightAngleMult(1.0F),
      mSubflares(),
      mRuntime() {}

// Reconstructed from eboot.elf at 0x47A600. The deleting destructor is at
// 0x47A760.
RndLightFlareCom::~RndLightFlareCom() {
    delete mRuntime.mOcclusionQuery;
    mRuntime.mOcclusionQuery = nullptr;
}

// Reconstructed from eboot.elf at 0x47BE20.
void RndLightFlareCom::_PostCreate() {
    Subflare subflare;
    subflare.mType = Subflare::kStarburst;
    mSubflares._Insert(mSubflares.size(), &subflare);
}

// Reconstructed from eboot.elf at 0x47BEF0. The query is named after the
// object.
bool RndLightFlareCom::_OnResourcesLoaded() {
    if (!RndDrawInstanceCom::_OnResourcesLoaded()) {
        return false;
    }
    if (mRuntime.mOcclusionQuery == nullptr) {
        mRuntime.mOcclusionQuery = RndOcclusionQuery::New(mObject->mName.Str());
    }
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    return true;
}

// Reconstructed from eboot.elf at 0x47BF80. The query is registered once;
// the manager gives it an index.
void RndLightFlareCom::_Enter() {
    _SyncSubflares();
    if (mRuntime.mOcclusionQuery->mFrame == -1) {
        _RegisterOcclusionQuery();
    }
    mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(
        Sphere{Vector3::sZero, mLightSourceRadius});
    RndDrawInstanceCom::_Enter();
}

// Reconstructed from eboot.elf at 0x47C120.
void RndLightFlareCom::_SyncSubflares() {
    eastl::vector<SubflareDrawable>& drawables = mRuntime.mSubflareDrawables;
    drawables.resize(mSubflares.size());
    for (unsigned long i = 0; i < drawables.size(); ++i) {
        drawables.mpBegin[i].mOwner = this;
        drawables.mpBegin[i].mIndex = i;
    }
}

// Reconstructed from eboot.elf at 0x47C220.
void RndLightFlareCom::_RegisterOcclusionQuery() {
    RndSceneDrawer* drawer = RndSceneDrawer::FindForEntity(mObject->mEntity);
    if (drawer != nullptr) {
        drawer->mOcclusionQueryMgr->RegisterQuery(*mRuntime.mOcclusionQuery);
    }
}

// Reconstructed from eboot.elf at 0x47C260. A hidden object, the options'
// "hide_flares" (unless the options are suppressed), or a hidden light
// hide the flare. A subtracting light (illumination type 3) leaves it
// hidden too, but still takes over the flare's light. The query is
// enabled with the flare, and a visible flare keeps the draw node's sphere
// on the light source and the query's sphere on its world sphere.
void RndLightFlareCom::_Poll() {
    mRuntime.mIsSpotLight = false;
    RndDrawNodeCom* drawNode = mObject->GetCom<RndDrawNodeCom>();
    const RndLightOptionsCom* options = theRndLightOpts;
    bool visible = false;
    const unsigned int flags = drawNode->mRuntime.mWorldShowHideFlags;
    if ((flags & RndDrawNodeCom::kHidden) == 0 &&
        (options->mRuntimeData.mSuppressed || !options->mHideFlares)) {
        RndLightCom* light = nullptr;
        if (mLight.mId != gNullObjectId.mId) {
            const GameObject* object = mObject->mEntity->GetObject(mLight);
            if (object != nullptr) {
                light = object->GetBaseCom<RndLightCom>();
            }
        }
        mRuntime.mLight = light;
        if (light == nullptr) {
            visible = true;
        } else if (light->mRuntime.mVisible) {
            visible = light->mIlluminationType != 3;
            if (visible) {
                mColor = light->mColor;
                mIntensity = light->mIntensity;
                const GameObject* lightObject = light->mObject;
                const auto* spot = lightObject->GetCom<RndLightSpotCom>();
                if (spot != nullptr) {
                    mRuntime.mIsSpotLight = true;
                    const Vector3& axis =
                        lightObject->GetExistingCom<TransCom>()->mWorldXfm.m.z;
                    const float inverse = InverseLength(axis.x, axis.y, axis.z);
                    mRuntime.mSpotDirection = Vector3{
                        inverse * axis.x, inverse * axis.y, inverse * axis.z};
                    mRuntime.mSpotFalloffParams =
                        spot->GetAngleFalloffParams(mSpotlightAngleMult);
                }
            }
        }
    }
    mRuntime.mVisible = visible;
    RndOcclusionQuery* query = mRuntime.mOcclusionQuery;
    query->mState[0] = visible;
    if (visible) {
        drawNode->SetLocalSphere(Sphere{Vector3::sZero, mLightSourceRadius});
        // The four words after mState hold the query's world sphere.
        std::memcpy(
            query->mResults, &drawNode->mRuntime.mWorldSphere, sizeof(Sphere));
    }
    RndDrawInstanceCom::_Poll();
}

// Reconstructed from eboot.elf at 0x47C5A0.
unsigned long RndLightFlareCom::_GetNumDrawInstancesImpl(
    RndSceneLod lod) const {
    if (((mLods >> (lod & 31)) & 1) == 0) {
        return 0;
    }
    return mRuntime.mSubflareDrawables.size();
}

// Reconstructed from eboot.elf at 0x47C5D0.
RndMaterialCom* RndLightFlareCom::_GetDefaultMaterial() const {
    return gRndDevice->mDefaults.mAdditiveMaterial;
}

// Reconstructed from eboot.elf at 0x47C5F0. Each instance takes the state
// flags of its subflare's material, or of the object's.
void RndLightFlareCom::_InitDrawInstancesImpl(
    VectorAdapter<RndDrawInstance>& instances) {
    const RndDrawInstanceCom::RuntimeData& base = RndDrawInstanceCom::mRuntime;
    const auto quality =
        static_cast<unsigned int>(gRndDevice->mSettings->mQualityLevel);
    RndMaterialCom* objectMaterial = ObjectMaterial(base);
    RndDrawInstance* list = Instances(instances);
    for (unsigned long i = 0; i < instances.mSize; ++i) {
        RndMaterialCom* material =
            FindMaterial(mObject->mEntity, mSubflares[i].mMaterial);
        if (material == nullptr) {
            material = objectMaterial;
        }
        const unsigned int hiddenFlag =
            (base.mDrawNode->mRuntime.mWorldShowHideFlags << 9) & 0x40000;
        list[i].mStateFlags =
            material->mRuntimeData->mUsageHints[quality] | hiddenFlag;
    }
}

// Reconstructed from eboot.elf at 0x47C8A0. A hidden flare clears the
// instances' drawables. Otherwise each instance draws its subflare with the
// object's world transform (transposed, with the translation in the last
// column), an identity normal transform, its material's render state and
// the draw node's sphere, clip planes and flags, without the atmosphere.
void RndLightFlareCom::_SyncDrawInstancesImpl(
    VectorAdapter<RndDrawInstance>& instances) {
    RndDrawInstance* list = Instances(instances);
    if (!mRuntime.mVisible) {
        for (unsigned long i = 0; i < instances.mSize; ++i) {
            list[i].mDrawable = nullptr;
        }
        return;
    }
    const RndDrawInstanceCom::RuntimeData& base = RndDrawInstanceCom::mRuntime;
    RndDevice* device = gRndDevice;
    const auto quality =
        static_cast<unsigned int>(device->mSettings->mQualityLevel);
    const Transform& xfm = base.mTrans->mWorldXfm;
    const Vector3* axes[3] = {&xfm.m.x, &xfm.m.y, &xfm.m.z};
    const Hmx::Matrix3& normal = Hmx::Matrix3::sID;
    const Vector3* normalAxes[3] = {&normal.x, &normal.y, &normal.z};
    RndMaterialCom* objectMaterial = ObjectMaterial(base);
    for (unsigned long i = 0; i < instances.mSize; ++i) {
        RndDrawInstance& instance = list[i];
        instance.mDrawable = &mRuntime.mSubflareDrawables.mpBegin[i];
        RndMaterialCom* material =
            FindMaterial(mObject->mEntity, mSubflares[i].mMaterial);
        float(&rows)[3][4] = instance.mInstanceData.mXfm;
        for (int axis = 0; axis < 3; ++axis) {
            rows[0][axis] = axes[axis]->x;
            rows[1][axis] = axes[axis]->y;
            rows[2][axis] = axes[axis]->z;
        }
        rows[0][3] = xfm.v.x;
        rows[1][3] = xfm.v.y;
        rows[2][3] = xfm.v.z;
        for (int axis = 0; axis < 3; ++axis) {
            instance.mInstanceData.mNormalXfm[0][axis] = normalAxes[axis]->x;
            instance.mInstanceData.mNormalXfm[1][axis] = normalAxes[axis]->y;
            instance.mInstanceData.mNormalXfm[2][axis] = normalAxes[axis]->z;
        }
        instance.mSortBy = mSortBy;
        if (material == nullptr) {
            material = objectMaterial;
        }
        instance.mSortingHint = material->mBlendMode == RndBlendMode::kSource
            ? static_cast<signed char>(mSortingHint)
            : 0;
        instance.mDebugTag = base.mActiveSortKey;
        instance.mInstanceData.mPackedState =
            static_cast<unsigned int>(mBillboarding);
        const float* params[2] = {&mExtraData.x, &mExtraData1.x};
        for (int set = 0; set < 2; ++set) {
            for (int component = 0; component < 4; ++component) {
                instance.mInstanceData.mParams[set][component] =
                    params[set][component];
            }
        }
        RndMaterialRuntimeData* data = material->mRuntimeData;
        data->mFrameStamp = device->mFrameCount;
        instance.mMaterial = data;
        instance.mUsesSceneTex = data->mRootFlags[1];
        instance.mUsesSceneDepth = data->mRootFlags[2];
        instance.mCullMode = material->mCullMode;
        instance.mReceiveAtmosphere = material->mReceiveAtmosphere;
        instance.mReceiveDecals = material->mReceiveDecals;
        const RndDrawNodeCom* node = base.mDrawNode;
        const unsigned int flags = node->mRuntime.mWorldShowHideFlags;
        instance.mStateFlags =
            data->mUsageHints[quality] | ((flags << 9) & 0x40000);
        instance.mWorldShowHideFlags = flags;
        instance.mCounterClockwise = node->mRuntime.mCounterClockwise;
        instance.mBounds = node->mRuntime.mWorldSphere;
        instance.mEnvironIndex = node->mRuntime.mEnvironIndex;
        instance.mClipPlanes[0] = node->mRuntime.mWorldClipPlanes[0];
        instance.mClipPlanes[1] = node->mRuntime.mWorldClipPlanes[1];
        instance.mReceiveAtmosphere = false;
    }
}

// Reconstructed from eboot.elf at 0x47CE60. The intensity follows a GGX
// distribution of the angle between the camera's view axis and the light
// source, whose roughness falls as "cam_centering_intensity" rises; it
// takes the light's master intensity, and a spotlight's angular falloff
// for the direction to the camera. A ghost dims as it grows; a starburst
// shows "intensity_scaling" of its intensity change as size instead. The
// quads keep the batch's shader and state, inside the query's predication.
void RndLightFlareCom::_DrawSubflare(
    RndContext& context,
    const RndInstanceData& instance,
    unsigned long index) {
    const Subflare& subflare = mSubflares[index];
    if (subflare.mScale <= 0.0F || subflare.mIntensityMult <= 0.0F) {
        return;
    }
    const long view = RndOcclusionQueryMgr::GetViewIndex(context.mTargetMode);
    RndOcclusionQuery* query = mRuntime.mOcclusionQuery;
    if (query->mState[1 + view] != 0) {
        return;
    }

    const Vector3 position = {
        instance.mXfm[0][3], instance.mXfm[1][3], instance.mXfm[2][3]};
    const Vector2 screen = context.mCameras[0].Project(
        position, k2DCoordHeightNormalized, nullptr);
    const Transform& camera = context.mCameras[0].mViews[0].mWorldXfm;
    const Vector3& source = mRuntime.mTrans->mWorldXfm.v;
    float dx = source.x - camera.v.x;
    float dy = source.y - camera.v.y;
    float dz = source.z - camera.v.z;
    const float inverse = InverseLength(dx, dy, dz);
    dx *= inverse;
    dy *= inverse;
    dz *= inverse;
    const Vector3& forward = camera.m.y;
    const float facing =
        std::max(0.0F, dx * forward.x + (dy * forward.y + dz * forward.z));

    const float centering = mCamCenteringIntensity;
    const float roughness = centering > 1.0F
        ? 0.06F
        : 1.0F - 0.94F * std::max(0.0F, centering);
    const float alpha = roughness * roughness;
    const float alphaSquared = alpha * alpha;
    const float denominator = facing * facing * (alphaSquared - 1.0F) + 1.0F;
    float intensity = subflare.mIntensityMult * mIntensity *
        (alphaSquared / (denominator * denominator * kPi));
    if (mRuntime.mLight != nullptr) {
        intensity *= mRuntime.mLight->mRuntime.mMasterIntensityMult;
    }
    if (mRuntime.mIsSpotLight) {
        const Vector3& direction = mRuntime.mSpotDirection;
        const Vector3& falloff = mRuntime.mSpotFalloffParams;
        const float cosine =
            dx * direction.x + (dy * direction.y + dz * direction.z);
        float t = cosine * falloff.x + falloff.y;
        t = t > 1.0F ? 1.0F : std::max(0.0F, t);
        intensity *= sample_function_table(
            static_cast<std::uint32_t>(RoundToInt(falloff.z)), t);
    }

    const float scale = subflare.mScale;
    if (subflare.mType == Subflare::kGhost) {
        intensity /= std::max(scale * scale, kMinDivisor);
    }
    if (std::fabs(intensity) <= kMinDivisor) {
        return;
    }
    float size = scale;
    if (subflare.mType == Subflare::kStarburst) {
        const float growth = (intensity / subflare.mIntensityMult - 1.0F) *
                subflare.mIntensityScaling +
            1.0F;
        size = growth * scale;
        intensity /= std::max(growth * growth, kMinDivisor);
    }

    query->SetShaderConstants(context);
    query->_BeginPredicationImpl(context);
    const float width = size * mSize.x;
    const float height = size * mSize.y;
    RndDrawUtl::RotatedQuad2DParams params;
    params.mCoordinateMode = RndDrawUtl::kCoordinateAspectCorrected;
    params.mRect.w = width;
    params.mRect.h = height;
    params.mColor = Hmx::Color(
        subflare.mTint.red * mColor.red,
        subflare.mTint.green * mColor.green,
        subflare.mTint.blue * mColor.blue,
        intensity);
    params.mRotation = mAngleRadians;
    params.mKeepShader = true;
    params.mKeepState = true;

    // The subflare diverges from the light source along the line through
    // the screen center, by "divergence" times a power of the source's
    // distance from the center.
    const float centerX = screen.x - 0.5F;
    const float centerY = screen.y - 0.5F;
    const float distance = std::sqrt(centerX * centerX + centerY * centerY);
    const float inverseDistance = 1.0F / std::max(distance, kMinDivisor);
    const float awayX = centerX * inverseDistance;
    const float awayY = centerY * inverseDistance;
    const float divergence = subflare.mDivergence * 0.25F *
        std::pow(distance * 4.0F, subflare.mDivergenceAccel);
    const float left = -0.5F * width + screen.x;
    const float top = -0.5F * height + screen.y;
    params.mRect.x = left + divergence * awayX;
    params.mRect.y = top + divergence * awayY;
    if (divergence == 0.0F) {
        params.mColor.alpha = intensity + intensity;
    } else {
        RndDrawUtl::DrawRotatedQuad2D(context, params);
        params.mRect.x = left - divergence * awayX;
        params.mRect.y = top - divergence * awayY;
    }
    RndDrawUtl::DrawRotatedQuad2D(context, params);
    query->_EndPredicationImpl(context);
}

// Reconstructed from eboot.elf at 0x47D4D0.
void RndLightFlareCom::SubflareDrawable::_DrawBatchImpl(
    RndContext& context,
    const VectorAdapter<RndInstanceData>& instances,
    const DrawRange& range) {
    static_cast<void>(range);
    mOwner->_DrawSubflare(context, instances.mData[0], mIndex);
}

// Reconstructed from eboot.elf at 0x47D4F0.
Symbol RndLightFlareCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x47D500.
Symbol RndLightFlareCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x47D510.
int RndLightFlareCom::CurrentRev() const {
    return const_cast<RndLightFlareCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x47D530.
bool RndLightFlareCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x47D560.
Component* RndLightFlareCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x47D570. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndLightFlareCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightFlareCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightFlareCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x47D7E0.
PropRegistry& RndLightFlareCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x47D7F0.
ComMetaData& RndLightFlareCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x405640. The binary emits the factory
// with the other render factories.
Component* RndLightFlareCom::_Create() {
    return new RndLightFlareCom();
}
