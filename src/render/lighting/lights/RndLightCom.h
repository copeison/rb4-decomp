#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "math/color/Color.h"
#include "math/geometry/Plane.h"
#include "math/vector/Vector4.h"
#include "utl/containers/VectorAdapter.h"
#include "render/system/RndConfig.h"
#include "utl/text/Symbol.h"

class ComMetaData;
class Entity;
class Frustum;
class PropRegistry;
class RndBufferCollection;
struct RndDrawInstance;
struct RndSceneDrawTarget;
struct RndShowHideContext;
class RndCameraContext;
class RndComputeBuffer;
class RndContext;
class RndDrawNodeCom;
class RndLightMgrCom;
class RndLightProbeCom;
class RndSceneDrawer;
class RndTextureBase;
class TransCom;
template <typename T>
class PodVector;

// The probe a light is drawn with in the untiled deferred pass: the scene's
// single probe and the two probe states it blends, which
// RndLightMgrCom::_AccumUntiledDeferredLight (0x4859B0) builds when
// "combine_single_probe" is set. The directional light's slots 44 and 45
// read it. Name not in the reference map.
struct RndLightProbeParams {
    RndLightProbeCom* mProbe;
    // The light manager's current probe-state indices (mCurProbeStateAIndex
    // and mCurProbeStateBIndex), or -1.
    unsigned long mStateIndices[2];
    float mStateBlend;
};

static_assert(offsetof(RndLightProbeParams, mStateBlend) == 24);
static_assert(sizeof(RndLightProbeParams) == 32);

// The extra planes a light's frustum test also checks, as (a, b, c, d)
// vectors: a view of the camera context's mDerivedCache, which
// RndLightMgrCom::_FrustumCullLights (0x482EB0) passes to slot 43. The
// alias name is not in the reference map; it keeps the light headers'
// declarations short.
using RndLightCullPlanes = VectorAdapter<Vector4>;

// The base of the light components (render/RndLightCom.o, 0x46C950 to
// 0x4705FF). Its class id is "Light"; the class is not creatable on its own
// ("Properties common to all lights, should not be directly creatable").
// A light registers with the light manager of its scene and with the light
// environments it names; the subclasses implement the drawing through the
// slots from 41. The vtable at 0x1904DC0 has 54 slots. The object is 232
// bytes. Field names are not in the reference map; the property members
// take the names of the properties the registry (0x46D760) binds to their
// offsets.
class RndLightCom : public Component {
public:
    // The runtime state after the properties (constructor 0x46CAC0, inlined
    // into RndLightCom's constructor). The map names the type; its field
    // names are not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x46CAC0
        // The map's RuntimeData(RuntimeData const&): an imprint's run-time
        // state starts afresh, so the copy is a default construction.
        // Inlined into RndLightCom's copy constructor.
        RuntimeData(const RuntimeData&) : RuntimeData() {}

        // The object's TransCom and RndDrawNodeCom, cached by
        // _OnResourcesLoaded.
        TransCom* mTrans;
        RndDrawNodeCom* mDrawNode;
        // The subclass's _GetCapabilitiesImpl, cached by
        // _OnResourcesLoaded.
        int mCapabilities;
        // Whether the light is on and its draw node is not hidden; set by
        // every _Poll.
        bool mVisible;
        // "environ_bits": the raw bits of the environments the light
        // contributes to. RndLightEnvironCom sets one bit per environment.
        unsigned int mEnvironBits;
        // Set while the light is registered with its light manager and
        // environments (_InitBookkeeping).
        bool mInBookkeeping;
        // Set when "environments" changed; the next _Poll registers the
        // light again.
        bool mBookkeepingDirty;
        // "active_environments": the environments the light registered with.
        PropArray<GameObjectId> mActiveEnvironments;
        // Set when "clip_plane" changed; the next _Poll finds its TransCom.
        bool mClipPlaneDirty;
        // The clip-plane object's TransCom, or null.
        TransCom* mClipPlaneTrans;
        // The world plane through the clip-plane object, facing its z axis;
        // all zero without a clip plane.
        Plane mClipPlane;
        // The light manager's "master_intensity_mult", copied by
        // RndLightMgrCom::AddLight (0x482810).
        float mMasterIntensityMult;
        // "isolate".
        bool mIsolate;
        // Cleared by the constructors; no reader was found.
        bool mReserved;
        // "tiled_cookie_index": the cookie's slice in its texture array, or
        // -1. RndLightMgrCom::_SyncCookieTexArrays (0x480F10) sets it.
        unsigned long mTiledCookieIndex;
        // The cookie texture array the slice is in, or -1.
        int mCookieTexArrayIndex;
        // The light's index in its type's light buffer, set by
        // RndLightMgrCom::_FillTiledLightBuffers (0x484160) before slot 46;
        // -1 after every _Poll.
        int mLightBufferIndex;
    };

    RndLightCom();  // 0x46C950
    // The map's RndLightCom(RndLightCom const&): copies the properties for
    // an imprint, "environments" through PropArrayBase::_Copy, and starts
    // the run-time state afresh. Inlined into _Imprint (0x46F490) and the
    // subclasses' copies.
    RndLightCom(const RndLightCom& other)
        : Component(other),
          mEnabled(other.mEnabled),
          mColor(other.mColor),
          mIntensity(other.mIntensity),
          mIlluminationType(other.mIlluminationType),
          mLightWrap(other.mLightWrap),
          mClipPlaneObject(other.mClipPlaneObject),
          mVolumetric(other.mVolumetric),
          mRuntime(other.mRuntime) {
        mEnvironments._Copy(other.mEnvironments);
    }

    // Slots 0-1: 0x46CB40, 0x46CC00.
    ~RndLightCom() override;
    Symbol GetId() const override;          // slot 4: 0x46F410
    Symbol GetClassName() const override;   // slot 5: 0x46F420
    int CurrentRev() const override;        // slot 7: 0x46F430
    bool IsA(Symbol type) const override;   // slot 8: 0x46F450
    Component* AsComponent() override;      // slot 9: 0x46F480
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x46F490
    // Slot 18 at 0x46EB90: lights follow RndDrawNodeCom in the component
    // order.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;
    // Slot 20 at 0x46EB40: leaves the bookkeeping; an exit of the whole
    // entity (types 0 and 1) only forgets it. The map's
    // _PreDestroy(ObjPtr const&, DestroyType).
    void _PreDestroy(DestroyType type) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x46F6B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x46F6C0
    // Slot 29 at 0x46E910: caches the capabilities and the object's
    // components, and finds the clip plane. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 30 at 0x46EB30: registers the light. Name not in the reference
    // map for this slot; the map's _InitBookkeeping is the body.
    bool _AreResourcesReady() override;
    // Slot 33 at 0x46EC50: follows the clip plane, updates the visibility
    // and registers the light again when its environments changed.
    void _Poll() override;
    // Slot 38 at 0x46EFE0: polls as in the game mode.
    void _EditPoll() override;

    // Slots 41-53: the light interface the subclasses implement. Most are
    // in the map; the parameters after the map's are this build's and their
    // types are inferred from the callers in RndLightMgrCom and
    // RndLightEnvironCom.
    // Slot 41 at 0x46F6D0: capability flags; 0 here and for directional
    // lights, 3 for point and spot lights.
    virtual int _GetCapabilitiesImpl() const;
    // Slot 42 at 0x46F6E0: the units of "intensity": "unknown units" here,
    // "W/m^2" for directional lights and "W/sr" for point and spot lights.
    // Name not in the reference map.
    virtual Symbol _GetIntensityUnitsImpl() const;
    // Slot 43 at 0x46F730: whether the frustum, or one of the extra planes,
    // excludes the light. False here.
    virtual bool _FrustumExcludesImpl(const Frustum& frustum, const RndLightCullPlanes& planes) const;
    // Slot 44 at 0x46F740: sets the light's constants before the untiled
    // deferred draw. Empty here.
    virtual void _PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe);
    // Slot 45 at 0x46F750: draws the light in the untiled deferred pass
    // (RndLightEnvironCom::DrawDeferredLightNoCompute, 0x479330). Empty
    // here. The target is the scene draw's (RndLightMgrCom::
    // AccumDeferredLight, 0x485270, passes it through).
    virtual void _DrawDeferredNoComputeImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        const RndLightProbeParams& probe);
    // Slot 46 at 0x46F760: appends the light to its type's light buffer.
    // Empty here.
    virtual void _AddToComputeBufferImpl(
        RndQualityLevel quality,
        const RndCameraContext& camera,
        RndComputeBuffer& buffer);
    // Slot 47 at 0x46F770: the light type: 0 point, 1 spot, 2 directional;
    // -1 here.
    virtual int _GetTypeImpl() const;
    // Slot 48 at 0x46F780: the cookie texture, or null. Null here.
    virtual RndTextureBase* _GetCookieTextureImpl() const;
    // Slot 49 at 0x46F790: whether the light casts shadows at the quality
    // level. False here.
    virtual bool _CastsShadowsImpl(RndQualityLevel quality) const;
    // Slot 50 at 0x46F7A0: draws the light's shadow map. Empty here.
    // RndLightMgrCom::DrawShadowMaps (0x485E40) passes through the scene's
    // show and hide flags and the scene drawer's instance lists.
    virtual void _DrawShadowMapImpl(
        RndContext& ctx,
        RndSceneDrawer& drawer,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
        RndLightMgrCom& mgr);
    // Slot 51 at 0x46F7B0: reserves the light's shadow-map and
    // shadow-contribution slots from the manager when `castsShadows` is set;
    // false when it has none. False here.
    virtual bool _AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr);
    // Slot 52 at 0x46F7C0: renders the light's shadow contribution into the
    // buffers for the scene draw's target. Empty here.
    virtual void _GenerateDeferredShadowContributionImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* sceneMask,
        RndLightMgrCom& mgr);
    // Slot 53 at 0x46F7D0: the subclass's part of IsOn. True here.
    virtual bool _IsOnImpl() const;

    // Whether the light is enabled, entered and bright: the intensity is
    // above 0.0001 and the color differs from the color that adds nothing
    // (white for illumination type 3, black otherwise); then the subclass
    // decides.
    bool IsOn() const;  // 0x46EE40
    // The clip plane in the light's draw-node space, normalized.
    // RndLightPointCom and RndLightSpotCom call it. Name not in the
    // reference map; the evidence is weak.
    Plane GetLocalClipPlane() const;  // 0x46D680

    // Registers the lights in the instanced entity of an instance
    // component, recursing into nested instances. The map's
    // AddNestedLightsToBookkeeping(ObjPtr const&); this build takes the
    // instance component.
    static void AddNestedLightsToBookkeeping(Component* instance);  // 0x46CD40
    // The reverse of AddNestedLightsToBookkeeping.
    static void RemoveNestedLightsFromBookkeeping(Component* instance);  // 0x46D390

    // Registers the light with its scene's light manager and with each
    // "environments" entry that is a RndLightEnvironCom, recording those in
    // "active_environments".
    void _InitBookkeeping();  // 0x46CE60
    // Unregisters the light from the manager and the active environments.
    void _RemoveFromBookkeeping();  // 0x46D4B0
    // Recomputes the clip plane from the clip-plane object's transform.
    void _SyncClipPlane();  // 0x46ED80
    // Finds the clip-plane object's TransCom and recomputes the plane.
    void _SyncClipPlaneTrans();  // 0x46E9D0
    // Marks the scene's cookie texture arrays for a rebuild.
    void _CookieTextureChanged();  // 0x46EFF0
    // The light manager of the nearest scene above the light.
    RndLightMgrCom* _FindActiveLightMgr() const;  // 0x46F090
    // Whether the light's own entity is a scene.
    bool _IsInScene() const;  // 0x46F120
    // The instance component that places the light's entity tree in its
    // scene, or null when the light's entity is the scene.
    Component* _FindInstanceInScene() const;  // 0x46F180
    // The entity whose objects the "environments" ids name: the light's own
    // entity when it is a scene, otherwise the entity that instances it in
    // the scene. Name not in the reference map.
    Entity* _FindEnvironmentEntity() const;  // 0x46F260
    // The light manager of the light's own scene.
    RndLightMgrCom* _FindSiblingLightMgr() const;  // 0x46F3A0

    // Not reconstructed: the property metadata, and the handlers at
    // 0x46F7E0-0x46FAD4 that it binds as std::function objects, are not
    // modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x46D760

    // The class symbol, "Light", constructed by the static initializer at
    // 0x470530.
    static Symbol sId;  // 0x1A873B0
    // The class's second symbol, also "Light". Base-class lookups compare
    // it with GameObject::ComIndex::mBaseId. Name not in the reference map.
    static Symbol sClassName;  // 0x1A873B8
    static PropRegistry sPropRegistry;  // 0x1A873C0
    static ComMetaData sMetaData;       // 0x1A87460

    // "enabled", placed in the Component base's tail padding.
    // RndDefaults::_LoadLighting (0x6BEF40) and _SyncEnabledLights
    // (0x6BFA60) write it.
    bool mEnabled;
    // "environments": the RndLightEnvironCom objects the light contributes
    // to.
    PropArray<GameObjectId> mEnvironments;
    Hmx::Color mColor;
    float mIntensity;
    // "illumination_type". Type 3 lights subtract: their color is a
    // multiplier, white adds nothing and the intensity stays below 1.
    int mIlluminationType;
    float mLightWrap;
    // "clip_plane": the object whose z axis clips the light, or the null id.
    GameObjectId mClipPlaneObject;
    // "volumetric": whether the light contributes to volumetric scattering.
    bool mVolumetric;
    // The map's RndLightCom::RuntimeData.
    RuntimeData mRuntime;
};

static_assert(offsetof(RndLightCom::RuntimeData, mCapabilities) == 0x10);
static_assert(offsetof(RndLightCom::RuntimeData, mVisible) == 0x14);
static_assert(offsetof(RndLightCom::RuntimeData, mEnvironBits) == 0x18);
static_assert(offsetof(RndLightCom::RuntimeData, mInBookkeeping) == 0x1C);
static_assert(offsetof(RndLightCom::RuntimeData, mActiveEnvironments) == 0x20);
static_assert(offsetof(RndLightCom::RuntimeData, mClipPlaneDirty) == 0x48);
static_assert(offsetof(RndLightCom::RuntimeData, mClipPlaneTrans) == 0x50);
static_assert(offsetof(RndLightCom::RuntimeData, mClipPlane) == 0x58);
static_assert(offsetof(RndLightCom::RuntimeData, mMasterIntensityMult) == 0x68);
static_assert(offsetof(RndLightCom::RuntimeData, mIsolate) == 0x6C);
static_assert(offsetof(RndLightCom::RuntimeData, mTiledCookieIndex) == 0x70);
static_assert(offsetof(RndLightCom::RuntimeData, mLightBufferIndex) == 0x7C);
static_assert(sizeof(RndLightCom::RuntimeData) == 0x80);
static_assert(offsetof(RndLightCom, mEnabled) == 22);
static_assert(offsetof(RndLightCom, mEnvironments) == 0x18);
static_assert(offsetof(RndLightCom, mColor) == 0x40);
static_assert(offsetof(RndLightCom, mIntensity) == 0x50);
static_assert(offsetof(RndLightCom, mIlluminationType) == 0x54);
static_assert(offsetof(RndLightCom, mLightWrap) == 0x58);
static_assert(offsetof(RndLightCom, mClipPlaneObject) == 0x5C);
static_assert(offsetof(RndLightCom, mVolumetric) == 0x60);
static_assert(offsetof(RndLightCom, mRuntime) == 0x68);
static_assert(sizeof(RndLightCom) == 0xE8);
