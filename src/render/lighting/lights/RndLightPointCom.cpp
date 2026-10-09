// render/RndLightPointCom.o (0x490060-0x496C9F). The PropArray element
// operations of "quality_settings" (vtable 0x1906D50), the element type's
// registration (0x492030), the registry's std::function handlers
// (0x4958D0-0x496C40) and the static initializer are not reconstructed.
#include "render/lighting/lights/RndLightPointCom.h"

#include <cmath>
#include <new>

#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/geometry/Sphere.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/lighting/RndLightGlobals.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/deferred/RndLightPointDeferredShader.h"
#include "render/lighting/shadows/RndShaderLightPointShadowGen.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureCube.h"
#include "render/textures/RndTextureCubeResource.h"

Symbol RndLightPointCom::sId("LightPoint");
Symbol RndLightPointCom::sClassName("LightPoint");
PropRegistry RndLightPointCom::sPropRegistry;
ComMetaData RndLightPointCom::sMetaData;

// Reconstructed from eboot.elf at 0x490060. The RuntimeData constructor is
// inlined here.
RndLightPointCom::RndLightPointCom()
    : mBulbRadius(0.05F),
      mFalloffStart(0.0F),
      mFalloffEnd(20.0F),
      mFalloffFunction(0),
      mCastsShadows(false),
      mOnlyFlaggedObjects(true),
      mShadowMapResolution(1),
      mShadowOffset(0.05F),
      mShadowSoftnessMin(0.1F),
      mShadowSoftnessMax(1.0F),
      mMaxShadowSoftnessDistance(5.0F) {}

// Reconstructed from eboot.elf at 0x4901E0.
RndLightPointCom::~RndLightPointCom() {
    RndShaderCBuffer::SafeDelete(mRuntimeData.mDeferredCBuffer);
    RndShaderCBuffer::SafeDelete(mRuntimeData.mShadowGenCBuffer);
    delete mRuntimeData.mShadowMap;
    mRuntimeData.mShadowMap = nullptr;
}

// Reconstructed from eboot.elf at 0x4921E0.
void RndLightPointCom::_PostCreate() {
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x492200.
void RndLightPointCom::_PreDestroy(DestroyType type) {
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

// Reconstructed from eboot.elf at 0x4922E0. The cookie is loaded through
// Resource::GetOrLoad<RndTextureCubeResource>, which the binary emits in
// this object at 0x4924E0; the map's _SyncLocalSphere is inlined.
bool RndLightPointCom::_OnResourcesLoaded() {
    RndLightCom::_OnResourcesLoaded();
    RndLightGlobals& lighting = TheRndDevice()->mLighting;
    if (mRuntimeData.mDeferredCBuffer == nullptr) {
        mRuntimeData.mDeferredCBuffer = RndShaderCBuffer::New(*lighting.mPointShader->mCBufferConfig, 0);
    }
    if (mRuntimeData.mShadowGenCBuffer == nullptr) {
        mRuntimeData.mShadowGenCBuffer =
            RndShaderCBuffer::New(*lighting.mPointShadowGenShader->mCBufferConfig, 0);
    }
    const ResourcePtr<RndTextureCubeResource> previous(mRuntimeData.mCookieTexture.Get());
    if (mCookie == ResourcePath()) {
        mRuntimeData.mCookieTexture = nullptr;
    } else {
        mRuntimeData.mCookieTexture = Resource::GetOrLoad<RndTextureCubeResource>(mCookie, false);
    }
    if (previous.Get() != mRuntimeData.mCookieTexture.Get()) {
        _CookieTextureChanged();
    }
    _CreateShadowMapData();
    mRuntimeData.mSphereDirty = false;
    mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(Sphere{Vector3::sZero, mFalloffEnd});
    return true;
}

// Reconstructed from eboot.elf at 0x492920.
void RndLightPointCom::_Poll() {
    RndLightCom::_Poll();
    if (mRuntimeData.mSphereDirty) {
        mRuntimeData.mSphereDirty = false;
        mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(Sphere{Vector3::sZero, mFalloffEnd});
    }
}

// Reconstructed from eboot.elf at 0x4929D0.
int RndLightPointCom::_GetCapabilitiesImpl() const {
    return 3;
}

// Reconstructed from eboot.elf at 0x4929E0.
Symbol RndLightPointCom::_GetIntensityUnitsImpl() const {
    return Symbol("W/sr");
}

// Reconstructed from eboot.elf at 0x493BE0.
RndTextureBase* RndLightPointCom::_GetCookieTextureImpl() const {
    RndTextureCubeResource* cookie = mRuntimeData.mCookieTexture.Get();
    if (cookie == nullptr || cookie->Fail()) {
        return nullptr;
    }
    return cookie->mTexture;
}

// Reconstructed from eboot.elf at 0x493C20.
bool RndLightPointCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    static_cast<void>(quality);
    return false;
}

// Reconstructed from eboot.elf at 0x493FB0.
bool RndLightPointCom::_AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) {
    long index = -1;
    if (castsShadows) {
        index = mgr.AcquireShadowContribution();
    }
    mRuntimeData.mShadowContributionIndex = index;
    return index != -1;
}

// Reconstructed from eboot.elf at 0x4954A0.
bool RndLightPointCom::_IsOnImpl() const {
    return std::fabs(mFalloffEnd) > 0.0001F;
}

// Reconstructed from eboot.elf at 0x4954C0.
void RndLightPointCom::_SyncFalloffStart() {
    const float end = mFalloffStart > mFalloffEnd ? mFalloffStart : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        mRuntimeData.mSphereDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x4954F0.
void RndLightPointCom::_SyncFalloffEnd() {
    mFalloffStart = mFalloffEnd < mFalloffStart ? mFalloffEnd : mFalloffStart;
    mRuntimeData.mSphereDirty = true;
}

// Reconstructed from eboot.elf at 0x4955A0.
Symbol RndLightPointCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4955B0.
Symbol RndLightPointCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4955C0.
int RndLightPointCom::CurrentRev() const {
    return const_cast<RndLightPointCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4955E0.
bool RndLightPointCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x495610.
Component* RndLightPointCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x495620. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightPointCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightPointCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightPointCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4958A0.
PropRegistry& RndLightPointCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4958B0.
ComMetaData& RndLightPointCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4958C0.
int RndLightPointCom::_GetTypeImpl() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x404660. The binary emits the factory
// with the class's Init (0x3F0120), before the renderer's components.
Component* RndLightPointCom::_Create() {
    return new RndLightPointCom();
}
