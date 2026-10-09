// render/RndLightCom.o (0x46C950 to 0x4705FF). The object also emits the
// std::function handlers that the registry binds to "environments",
// "illumination_type", "clip_plane", "is_on", "has_zero_color" and
// "has_zero_intensity" (0x46F7E0-0x4704C0); they are not modelled.
#include "render/lighting/lights/RndLightCom.h"

#include <cmath>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropRegistry.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/lights/RndLightEnvironCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneCom.h"

// The class symbol of the instance component, "Instance" (sId 0x19E3278,
// vtable 0x18E6D58), which EntityResource.o also uses. Name not in the
// reference map.
extern Symbol gEntityInstanceComId;  // 0x19E3280

namespace {

// The id of no object, at 0x1A873A0, which the static initializer (0x470530)
// sets with the shared header's thread-group widths. Each object that uses
// it has its own copy. Names not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};
[[maybe_unused]] int gLightComGroupSize2D = 8;  // 0x1A873A4
[[maybe_unused]] int gLightComGroupSize3D = 4;  // 0x1A873A8

// A color channel matches when it is within this of the reference.
constexpr float kColorEpsilon = 0.0001F;

// The object's instance component, found by its class symbol as
// GameObject::GetBaseCom does. Inlined into its users. Name not in the
// reference map.
Component* FindInstanceCom(const GameObject* object) {
    if (gEntityInstanceComId == Symbol()) {
        return nullptr;
    }
    for (const GameObject::ComIndex& index : object->mComs) {
        if (index.mBaseId == gEntityInstanceComId) {
            return index.mCom;
        }
    }
    return nullptr;
}

// The entity an instance component instances, at its offset 80. The class
// is not modelled. Name not in the reference map.
Entity* GetInstancedEntity(const Component* instance) {
    return *reinterpret_cast<Entity* const*>(reinterpret_cast<const char*>(instance) + 80);
}

// The scene object above the object: the first object, from the object up
// through the objects that instance its entity, whose entity's root holds a
// RndSceneCom. Inlined into its users. Name not in the reference map.
GameObject* FindSceneObject(GameObject* object, RndSceneCom** scene) {
    for (; object != nullptr; object = object->mEntity->mParentObject) {
        if (RndSceneCom* found = object->mEntity->GetRoot()->GetCom<RndSceneCom>()) {
            *scene = found;
            return object;
        }
    }
    return nullptr;
}

bool ColorMatches(const Hmx::Color& color, const Hmx::Color& reference) {
    return std::fabs(color.red - reference.red) < kColorEpsilon &&
        std::fabs(color.green - reference.green) < kColorEpsilon &&
        std::fabs(color.blue - reference.blue) < kColorEpsilon;
}

// The reciprocal of the length, or 0 for a zero vector.
float InverseLength(float x, float y, float z) {
    const float length = std::sqrt(x * x + y * y + z * z);
    return length != 0.0F ? 1.0F / length : 0.0F;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x470530).
Symbol RndLightCom::sId("Light");
Symbol RndLightCom::sClassName("Light");
PropRegistry RndLightCom::sPropRegistry;
ComMetaData RndLightCom::sMetaData;

// Reconstructed from eboot.elf at 0x46C950.
RndLightCom::RndLightCom()
    : mEnabled(true),
      mColor(Hmx::Color::GetWhite()),
      mIntensity(1.0F),
      mIlluminationType(0),
      mLightWrap(1.0F),
      mClipPlaneObject(gNullObjectId),
      mVolumetric(false) {}

// Reconstructed from eboot.elf at 0x46CAC0. The capabilities are left
// unset until _OnResourcesLoaded.
RndLightCom::RuntimeData::RuntimeData()
    : mTrans(nullptr),
      mDrawNode(nullptr),
      mVisible(false),
      mEnvironBits(0),
      mInBookkeeping(false),
      mBookkeepingDirty(false),
      mClipPlaneDirty(false),
      mClipPlaneTrans(nullptr),
      mMasterIntensityMult(1.0F),
      mIsolate(false),
      mReserved(false),
      mTiledCookieIndex(static_cast<unsigned long>(-1)),
      mCookieTexArrayIndex(-1),
      mLightBufferIndex(-1) {
    mClipPlane.a = 0.0F;
    mClipPlane.b = 0.0F;
    mClipPlane.c = 0.0F;
    mClipPlane.d = 0.0F;
}

// Reconstructed from eboot.elf at 0x46CB40. The arrays release their
// storage.
RndLightCom::~RndLightCom() {}

// Reconstructed from eboot.elf at 0x46CD40.
void RndLightCom::AddNestedLightsToBookkeeping(Component* instance) {
    const Entity* entity = GetInstancedEntity(instance);
    if (entity == nullptr) {
        return;
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (RndLightCom* light = object->GetBaseCom<RndLightCom>()) {
            light->_InitBookkeeping();
        }
        if (Component* nested = FindInstanceCom(object)) {
            AddNestedLightsToBookkeeping(nested);
        }
    }
}

// Reconstructed from eboot.elf at 0x46CE60. A light in a scene's own entity
// names environments of that entity; a light in an instanced entity uses
// the instancing object's "nested_light_environments", which name objects
// of the instancing entity.
void RndLightCom::_InitBookkeeping() {
    mRuntime.mInBookkeeping = true;
    if (RndLightMgrCom* mgr = _FindActiveLightMgr()) {
        mgr->AddLight(this);
    }
    mRuntime.mBookkeepingDirty = false;

    const PropArray<GameObjectId>* environments;
    const Entity* entity;
    if (_IsInScene()) {
        environments = &mEnvironments;
        entity = mObject->mEntity;
    } else {
        RndSceneCom* scene = nullptr;
        GameObject* sceneObject = FindSceneObject(mObject, &scene);
        if (sceneObject == nullptr) {
            return;
        }
        Component* instance = FindInstanceCom(sceneObject);
        if (instance == nullptr) {
            return;
        }
        RndDrawNodeCom* node = instance->mObject->GetCom<RndDrawNodeCom>();
        if (node == nullptr) {
            return;
        }
        environments = &node->mNestedLightEnvironments;
        entity = instance->mObject->mEntity;
    }

    mRuntime.mActiveEnvironments.Reserve(environments->size());
    for (unsigned int i = 0; i < environments->size(); ++i) {
        const GameObjectId id = (*environments)[i];
        if (id.mId == gNullObjectId.mId) {
            continue;
        }
        GameObject* object = entity->GetObject(id);
        if (object == nullptr) {
            continue;
        }
        if (RndLightEnvironCom* environ = object->GetCom<RndLightEnvironCom>()) {
            environ->LightAdded(this);
            mRuntime.mActiveEnvironments._Insert(mRuntime.mActiveEnvironments.size(), &id);
        }
    }
}

// Reconstructed from eboot.elf at 0x46D390.
void RndLightCom::RemoveNestedLightsFromBookkeeping(Component* instance) {
    const Entity* entity = GetInstancedEntity(instance);
    if (entity == nullptr) {
        return;
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (RndLightCom* light = object->GetBaseCom<RndLightCom>()) {
            light->_RemoveFromBookkeeping();
        }
        if (Component* nested = FindInstanceCom(object)) {
            RemoveNestedLightsFromBookkeeping(nested);
        }
    }
}

// Reconstructed from eboot.elf at 0x46D4B0.
void RndLightCom::_RemoveFromBookkeeping() {
    if (!mRuntime.mInBookkeeping) {
        return;
    }
    mRuntime.mInBookkeeping = false;
    if (RndLightMgrCom* mgr = _FindActiveLightMgr()) {
        mgr->RemoveLight(this);
    }
    if (const Entity* entity = _FindEnvironmentEntity()) {
        for (unsigned int i = 0; i < mRuntime.mActiveEnvironments.size(); ++i) {
            const GameObjectId id = mRuntime.mActiveEnvironments[i];
            if (id.mId == gNullObjectId.mId) {
                continue;
            }
            GameObject* object = entity->GetObject(id);
            if (object == nullptr) {
                continue;
            }
            if (RndLightEnvironCom* environ = object->GetCom<RndLightEnvironCom>()) {
                environ->LightRemoved(this);
            }
        }
    }
    mRuntime.mActiveEnvironments.Resize(0);
    mRuntime.mEnvironBits = 0;
}

// Reconstructed from eboot.elf at 0x46D680.
Plane RndLightCom::GetLocalClipPlane() const {
    Plane plane;
    Multiply(mRuntime.mClipPlane, mRuntime.mDrawNode->mRuntime.mInvWorldXfm, plane);
    const float scale = InverseLength(plane.a, plane.b, plane.c);
    plane.a *= scale;
    plane.b *= scale;
    plane.c *= scale;
    plane.d *= scale;
    return plane;
}

// Reconstructed from eboot.elf at 0x46E910.
bool RndLightCom::_OnResourcesLoaded() {
    mRuntime.mCapabilities = _GetCapabilitiesImpl();
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    _SyncClipPlaneTrans();
    return true;
}

// Reconstructed from eboot.elf at 0x46E9D0. The clip-plane object must
// have a TransCom.
void RndLightCom::_SyncClipPlaneTrans() {
    mRuntime.mClipPlaneDirty = false;
    if (mClipPlaneObject.mId == gNullObjectId.mId) {
        mRuntime.mClipPlaneTrans = nullptr;
        mRuntime.mClipPlane.a = 0.0F;
        mRuntime.mClipPlane.b = 0.0F;
        mRuntime.mClipPlane.c = 0.0F;
        mRuntime.mClipPlane.d = 0.0F;
        return;
    }
    mRuntime.mClipPlaneTrans = mObject->mEntity->GetObject(mClipPlaneObject)->GetCom<TransCom>();
    _SyncClipPlane();
}

// Reconstructed from eboot.elf at 0x46EB30.
bool RndLightCom::_AreResourcesReady() {
    _InitBookkeeping();
    return true;
}

// Reconstructed from eboot.elf at 0x46EB40.
void RndLightCom::_PreDestroy(DestroyType type) {
    if (type >= kDestroyInstance) {
        _RemoveFromBookkeeping();
        return;
    }
    mRuntime.mInBookkeeping = false;
    mRuntime.mActiveEnvironments.Resize(0);
    mRuntime.mEnvironBits = 0;
}

// Reconstructed from eboot.elf at 0x46EB90.
void RndLightCom::_GetComponentOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(RndDrawNodeCom::sClassName);
}

// Reconstructed from eboot.elf at 0x46EC50. Bit 2 of the TransCom's flags
// marks a changed world transform.
void RndLightCom::_Poll() {
    if (mRuntime.mClipPlaneDirty) {
        _SyncClipPlaneTrans();
    } else if (mRuntime.mClipPlaneTrans != nullptr && (mRuntime.mClipPlaneTrans->mDirtyFlags & 2) != 0) {
        _SyncClipPlane();
    }
    mRuntime.mLightBufferIndex = -1;
    mRuntime.mVisible = false;
    if (IsOn()) {
        mRuntime.mVisible = (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) == 0;
    }
    if (mRuntime.mBookkeepingDirty) {
        _RemoveFromBookkeeping();
        _InitBookkeeping();
    }
}

// Reconstructed from eboot.elf at 0x46ED80. The plane faces the object's z
// axis through its position.
void RndLightCom::_SyncClipPlane() {
    const Transform& xfm = mRuntime.mClipPlaneTrans->mWorldXfm;
    const Vector3& axis = xfm.m.z;
    const float scale = InverseLength(axis.x, axis.y, axis.z);
    Plane& plane = mRuntime.mClipPlane;
    plane.a = scale * axis.x;
    plane.b = scale * axis.y;
    plane.c = scale * axis.z;
    plane.d = -(plane.a * xfm.v.x + plane.b * xfm.v.y + plane.c * xfm.v.z);
}

// Reconstructed from eboot.elf at 0x46EE40.
bool RndLightCom::IsOn() const {
    if (!mEnabled || !mEntered || kColorEpsilon > mIntensity) {
        return false;
    }
    const Hmx::Color& noLight = mIlluminationType == 3 ? Hmx::Color::GetWhite() : Hmx::Color::GetBlack();
    if (ColorMatches(mColor, noLight)) {
        return false;
    }
    return _IsOnImpl();
}

// Reconstructed from eboot.elf at 0x46EFE0.
void RndLightCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x46EFF0.
void RndLightCom::_CookieTextureChanged() {
    if (!mRuntime.mInBookkeeping) {
        return;
    }
    if (RndLightMgrCom* mgr = _FindActiveLightMgr()) {
        mgr->mCookieTexArraysDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x46F090.
RndLightMgrCom* RndLightCom::_FindActiveLightMgr() const {
    RndSceneCom* scene = nullptr;
    if (FindSceneObject(mObject, &scene) == nullptr) {
        return nullptr;
    }
    return scene->GetLightMgr();
}

// Reconstructed from eboot.elf at 0x46F120.
bool RndLightCom::_IsInScene() const {
    return mObject->mEntity->GetRoot()->GetCom<RndSceneCom>() != nullptr;
}

// Reconstructed from eboot.elf at 0x46F180.
Component* RndLightCom::_FindInstanceInScene() const {
    RndSceneCom* scene = nullptr;
    GameObject* sceneObject = FindSceneObject(mObject, &scene);
    if (sceneObject == nullptr) {
        return nullptr;
    }
    return FindInstanceCom(sceneObject);
}

// Reconstructed from eboot.elf at 0x46F260.
Entity* RndLightCom::_FindEnvironmentEntity() const {
    if (_IsInScene()) {
        return mObject->mEntity;
    }
    const Component* instance = _FindInstanceInScene();
    return instance != nullptr ? instance->mObject->mEntity : nullptr;
}

// Reconstructed from eboot.elf at 0x46F3A0.
RndLightMgrCom* RndLightCom::_FindSiblingLightMgr() const {
    RndSceneCom* scene = mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    return scene != nullptr ? scene->GetLightMgr() : nullptr;
}

// Reconstructed from eboot.elf at 0x46F410.
Symbol RndLightCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x46F420.
Symbol RndLightCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x46F430.
int RndLightCom::CurrentRev() const {
    return const_cast<RndLightCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x46F450.
bool RndLightCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x46F480.
Component* RndLightCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x46F490. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x46F6B0.
PropRegistry& RndLightCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x46F6C0.
ComMetaData& RndLightCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x46F6D0.
int RndLightCom::_GetCapabilitiesImpl() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x46F6E0.
Symbol RndLightCom::_GetIntensityUnitsImpl() const {
    return Symbol("unknown units");
}

// Reconstructed from eboot.elf at 0x46F730.
bool RndLightCom::_FrustumExcludesImpl(const Frustum& frustum, const RndLightCullPlanes& planes) const {
    static_cast<void>(frustum);
    static_cast<void>(planes);
    return false;
}

// Reconstructed from eboot.elf at 0x46F740.
void RndLightCom::_PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) {
    static_cast<void>(ctx);
    static_cast<void>(probe);
}

// Reconstructed from eboot.elf at 0x46F750.
void RndLightCom::_DrawDeferredNoComputeImpl(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    const RndLightProbeParams& probe) {
    static_cast<void>(ctx);
    static_cast<void>(buffers);
    static_cast<void>(target);
    static_cast<void>(probe);
}

// Reconstructed from eboot.elf at 0x46F760.
void RndLightCom::_AddToComputeBufferImpl(
    RndQualityLevel quality,
    const RndCameraContext& camera,
    RndComputeBuffer& buffer) {
    static_cast<void>(quality);
    static_cast<void>(camera);
    static_cast<void>(buffer);
}

// Reconstructed from eboot.elf at 0x46F770.
int RndLightCom::_GetTypeImpl() const {
    return -1;
}

// Reconstructed from eboot.elf at 0x46F780.
RndTextureBase* RndLightCom::_GetCookieTextureImpl() const {
    return nullptr;
}

// Reconstructed from eboot.elf at 0x46F790.
bool RndLightCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    static_cast<void>(quality);
    return false;
}

// Reconstructed from eboot.elf at 0x46F7A0.
void RndLightCom::_DrawShadowMapImpl(
    RndContext& ctx,
    RndSceneDrawer& drawer,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& instances,
    VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
    RndLightMgrCom& mgr) {
    static_cast<void>(ctx);
    static_cast<void>(drawer);
    static_cast<void>(camera);
    static_cast<void>(showHide);
    static_cast<void>(instances);
    static_cast<void>(sortable);
    static_cast<void>(mgr);
}

// Reconstructed from eboot.elf at 0x46F7B0.
bool RndLightCom::_AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) {
    static_cast<void>(castsShadows);
    static_cast<void>(mgr);
    return false;
}

// Reconstructed from eboot.elf at 0x46F7C0.
void RndLightCom::_GenerateDeferredShadowContributionImpl(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    RndTextureBase* sceneMask,
    RndLightMgrCom& mgr) {
    static_cast<void>(ctx);
    static_cast<void>(buffers);
    static_cast<void>(target);
    static_cast<void>(sceneMask);
    static_cast<void>(mgr);
}

// Reconstructed from eboot.elf at 0x46F7D0.
bool RndLightCom::_IsOnImpl() const {
    return true;
}
